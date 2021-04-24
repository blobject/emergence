#include "proc.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <GL/glew.h>


Proc::Proc(Log& log, State& state, Cl& cl, bool no_cl)
  : state_(state), cl_(cl)
{
  this->cl_good_ = this->cl_.good();
  if (no_cl) {
    this->cl_good_ = false;
  }
  if (!this->cl_good_) {
    log.add(Attn::O, "Proceeding without OpenCL parallelisation.", true);
  }
  log.add(Attn::O, "Started process module.", true);
}


void
Proc::next()
{
  this->clear();

#if 1 == CL_ENABLED

  if (this->cl_good_) {
    this->seek();
    this->move();
    this->notify(Issue::ProcNextDone); // Views react
    return;
  }

#endif /* CL_ENABLED */

  this->plain_seek(this->state_.scope_, this->grid_,
                   this->grid_cols_, this->grid_rows_, this->grid_stride_,
                   &Proc::tally_neighborhood);
  this->plain_move();
  this->notify(Issue::ProcNextDone); // Views react
}


void
Proc::clear()
{
  State& state = this->state_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<unsigned int>& pl = state.pl_;
  std::vector<unsigned int>& pr = state.pr_;
  std::vector<unsigned int>& pan = state.pan_;
  std::vector<int>& pls = state.pls_;
  std::vector<int>& prs = state.prs_;
  std::vector<float>& pld = state.pld_;
  std::vector<float>& prd = state.prd_;
  unsigned int n_stride = state.n_stride_;
  unsigned int istride;
  unsigned int jstride;

  for (int i = 0; i < state.num_; ++i) {
    pn[i] = 0;
    pl[i] = 0;
    pr[i] = 0;
    pan[i] = 0;
    istride = n_stride * i;
    for (int j = 0; j < n_stride; ++j) {
      jstride = istride + j;
      if (0 > pls[jstride]) {
        break;
      }
      pls[jstride] = -1;
      pld[jstride] = -1.0f;
    }
    for (int j = 0; j < n_stride; ++j) {
      jstride = istride + j;
      if (0 > prs[jstride]) {
        break;
      }
      prs[jstride] = -1;
      prd[jstride] = -1.0f;
    }
  }
}


void
Proc::plot(unsigned int scope, std::vector<int>& grid, int& cols, int& rows,
           unsigned int& stride)
{
  State& state = this->state_;
  unsigned int num = state.num_;
  float width = state.width_;
  float height = state.height_;
  // the columns and rows of the grid are semi-flattened to a single list, but
  // each element is itself a list (of particle indices) -- "semi-" because the
  // entire thing will be further flattened later
  cols = 1; if (width  > scope) { cols = floor(width  / scope); }
  rows = 1; if (height > scope) { rows = floor(height / scope); }
  unsigned int unflat_size = cols * rows;
  auto unflat = std::vector<std::vector<int>>(unflat_size);
  float unit_width = state.width_ / cols;
  float unit_height = state.height_ / rows;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<int>& gcol = state.gcol_;
  std::vector<int>& grow = state.grow_;
  gcol.resize(num);
  grow.resize(num);

  int col;
  int row;
  for (int i = 0; i < num; ++i) {
    // the last column/row may be spatially slightly bigger than the rest,
    // if the grid does not divide neatly into whole numbers
    col = floor(px[i] / unit_width);  if (col >= cols) { col = cols - 1; }
    row = floor(py[i] / unit_height); if (row >= rows) { row = rows - 1; }
    gcol[i] = col;
    grow[i] = row;
    unflat[cols * row + col].push_back(i);
  }

  // find the size of the largest grid unit
  stride = 0;
  unsigned int count = 0;
  for (unsigned int i = 0; i < unflat_size; ++i) {
    count = unflat[i].size();
    if (count > stride) {
      stride = count;
    }
  }

  // flatten the grid (a single list of particle indices, padding with -1)
  auto flat = std::vector<int>();
  for (std::vector<int> unit : unflat) {
    count = 0;
    for (int p : unit) {
      flat.push_back(p);
      ++count;
    }
    while (count < stride) {
      flat.push_back(-1);
      ++count;
    }
  }

  grid = flat;
}


#if 1 == CL_ENABLED

void
Proc::seek()
{
  State& state = this->state_;
  this->plot(state.scope_, this->grid_,
             this->grid_cols_, this->grid_rows_, this->grid_stride_);
  this->cl_.seek(state.num_, state.width_, state.height_,
                 state.scope_squared_, state.ascope_squared_,
                 this->grid_cols_, this->grid_rows_,
                 this->grid_stride_, this->grid_, state.gcol_, state.grow_,
                 state.px_, state.py_, state.pc_, state.ps_,
                 state.pn_, state.pan_, state.pl_, state.pr_);
}


void
Proc::move()
{
  State& state = this->state_;
  this->cl_.move(state.num_, state.width_, state.height_,
                 state.alpha_, state.beta_, state.speed_, state.noise_,
                 state.pn_, state.pl_, state.pr_,
                 state.px_, state.py_, state.pf_, state.pc_, state.ps_);
}

#endif /* CL_ENABLED */


void
Proc::plain_seek(unsigned int scope, std::vector<int>& grid,
                 int& cols, int& rows, unsigned int& stride,
                 void (Proc::*tally)(int,int,float,float,float))
{
  State& state = this->state_;
  unsigned int num = state.num_;
  std::vector<int>& gcol = state.gcol_;
  std::vector<int>& grow = state.grow_;
  unsigned int scopesq = scope * scope;

  this->plot(scope, grid, cols, rows, stride);

  // for each particle index
  for (int srci = 0; srci < num; ++srci) {
    this->plain_seek_vicinity(scopesq, grid, stride, gcol[srci], grow[srci],
                              cols, rows, srci, tally);
  }
  for (int i = 0; i < num; ++i) {
    state.pn_[i] = state.pl_[i] + state.pr_[i];
  }
}


void
Proc::plain_seek_vicinity(unsigned int scopesq, std::vector<int>& grid,
                          unsigned int gstride,
                          int col, int row, int cols, int rows, int srci,
                          void (Proc::*tally)(int,int,float,float,float))
{
  // recognise the vicinity (with edge wrapping)
  int c = col - 1;
  int cc = col + 1;
  int r = row - 1;
  int rr = row + 1;
  bool cunder = false;
  bool cover = false;
  bool runder = false;
  bool rover = false;
  if      (col == 0)        { cunder = true; c = cols - 1; }
  else if (col == cols - 1) { cover  = true; cc = 0; }
  if      (row == 0)        { runder = true; r = rows - 1; }
  else if (row == rows - 1) { rover  = true; rr = 0; }
  // NOTE 1: World origin is southwest, so top-bottom order is reversed. This
  //         does not affect any core calculations, but helps us match up
  //         with the flattened grid generated by Plot().
  // NOTE 2: Size is 3x3(# units) * 6(col,row,col_under/over,row_under/over).
  int vic[54] = {/* sw */ c,   r,   cunder, false, runder, false,
                 /* s  */ col, r,   false,  false, runder, false,
                 /* se */ cc,  r,   false,  cover, runder, false,
                 /* w  */ c,   row, cunder, false, false,  false,
                 /* c  */ col, row, false,  false, false,  false,
                 /* e  */ cc,  row, false,  cover, false,  false,
                 /* nw */ c,   rr,  cunder, false, false,  rover,
                 /* n  */ col, rr,  false,  false, false,  rover,
                 /* ne */ cc,  rr,  false,  cover, false,  rover};
  unsigned int stride;
  int dsti;

  // for every unit in the vicinity (neighborhood)
  for (unsigned int v = 0; v < 54; v += 6) {
    stride = (cols * (vic[v + 1] * gstride)) + (vic[v] * gstride);
    // for each particle index within the unit
    for (unsigned int p = 0; p < gstride; ++p) {
      dsti = grid[stride + p];
      // avoid grid padding area (meaning no particle left in that unit)
      if (dsti == -1) {
        break;
      }
      // avoid redundant calculations
      if (srci <= dsti) {
        continue;
      }
      this->plain_seek_tally(scopesq, srci, dsti,
                             vic[v + 2], vic[v + 3], vic[v + 4], vic[v + 5],
                             tally);
    }
  }
}


void
Proc::plain_seek_tally(unsigned int scopesq,
                       unsigned int srci, unsigned int dsti,
                       bool cunder, bool cover, bool runder, bool rover,
                       void (Proc::*tally)(int,int,float,float,float))
{
  State& state = this->state_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  float srcx = px[srci];
  float srcy = py[srci];
  float dstx = px[dsti];
  float dsty = py[dsti];
  float width = state.width_;
  float height = state.height_;
  float dx = dstx - srcx;
  if (cunder) { dx -= width; }  else if (cover) { dx += width; }
  float dy = dsty - srcy;
  if (runder) { dy -= height; } else if (rover) { dy += height; }
  float distsq = (dx * dx) + (dy * dy);

  // ignore comparisons outside the vicinity scope
  if (scopesq < distsq) {
    return;
  }

  (this->*tally)(srci, dsti, dx, dy, distsq);
}


void
Proc::plain_move()
{
  State& state = this->state_;
  float width = state.width_;
  float height = state.height_;
  float alpha = state.alpha_;
  float beta = state.beta_;
  float speed = state.speed_;
  float noise = state.noise_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& pf = state.pf_;
  std::vector<float>& pc = state.pc_;
  std::vector<float>& ps = state.ps_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<unsigned int>& pl = state.pl_;
  std::vector<unsigned int>& pr = state.pr_;
  float f;
  float x;
  float y;

  for (int i = 0; i < state.num_; ++i) {
    f = fmod(pf[i] + alpha
             + (beta * pn[i]
                * Util::signum(static_cast<int>(pr[i] - pl[i]))), TAU)
             + noise;
    if (f < 0) { f += TAU; }
    pf[i] = f;
    pc[i] = cosf(f);
    ps[i] = sinf(f);
    x = fmod(px[i] + speed * pc[i], width);
    if (x < 0) { x += width; }
    px[i] = x;
    y = fmod(py[i] + speed * ps[i], height);
    if (y < 0) { y += height; }
    py[i] = y;
  }
}


void
Proc::tally_neighborhood(int srci, int dsti, float dx, float dy, float distsq)
{
  State& state = this->state_;
  std::vector<float>& pc = state.pc_;
  std::vector<float>& ps = state.ps_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<unsigned int>& pl = state.pl_;
  std::vector<unsigned int>& pr = state.pr_;
  std::vector<unsigned int>& pan = state.pn_;
  std::vector<int>& pls = state.pls_;
  std::vector<int>& prs = state.prs_;
  std::vector<float>& pld = state.pld_;
  std::vector<float>& prd = state.prd_;
  unsigned int n_stride = state.n_stride_;

  float srcc = pc[srci];
  float srcs = ps[srci];
  float dstc = pc[dsti];
  float dsts = ps[dsti];
  unsigned int srcstride = n_stride * srci;
  unsigned int dststride = n_stride * dsti;
  unsigned int srcl = pl[srci];
  unsigned int srcr = pr[srci];
  unsigned int dstl = pl[dsti];
  unsigned int dstr = pr[dsti];
  unsigned int srcli = srcstride + srcl;
  unsigned int srcri = srcstride + srcr;
  unsigned int dstli = dststride + dstl;
  unsigned int dstri = dststride + dstr;

  ++pn[srci];
  ++pn[dsti];

  if (0.0f > (dx * srcs) - (dy * srcc)) {
    if (n_stride > srcr) {
      prs[srcri] = dsti;
      prd[srcri] = distsq;
    }
    ++pr[srci];
  } else {
    if (n_stride > srcl) {
      pls[srcli] = dsti;
      pld[srcli] = distsq;
    }
    ++pl[srci];
  }
  if (0.0f < (dx * dsts) - (dy * dstc)) {
    if (n_stride > dstr) {
      prs[dstri] = srci;
      prd[dstri] = distsq;
    }
    ++pr[dsti];
  } else {
    if (n_stride > dstl) {
      pls[dstli] = srci;
      pld[dstli] = distsq;
    }
    ++pl[dsti];
  }
}


void
Proc::tally_neighbors(int srci, int dsti, float /* dx */, float /* dy */,
                      float distsq)
{
  std::unordered_map<int,std::vector<int>>& ns = this->neighbors_sets_;
  std::unordered_map<int,std::vector<float>>& nd = this->neighbors_dists_;
  float dist = std::sqrt(distsq);

  ns[srci].push_back(dsti);
  ns[dsti].push_back(srci);
  nd[srci].push_back(dist);
  nd[dsti].push_back(dist);
}

