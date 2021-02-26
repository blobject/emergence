#include "proc.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <GL/glew.h>


Proc::Proc(Log& log, State& state, Cl& cl, bool no_cl)
    : state_(state), cl_(cl)
{
    this->paused_ = false;
    this->cl_good_ = this->cl_.good();
    if (no_cl) {
      this->cl_good_ = false;
    }
    if (!this->cl_good_) {
        std::string message = "Proceeding without OpenCL parallelisation.";
        log.add(Attn::O, "Proceeding without OpenCL parallelisation.", true);
    }
}


void
Proc::next()
{
    if (this->paused_) {
        // do no processing but still let Views do their thing
        this->notify(Issue::ProcNextDone); // Views react
        return;
    }
    this->plot();

#ifdef HAS_CL

    if (this->cl_good_) {
        this->seek();
        this->move();
        this->notify(Issue::ProcNextDone); // Views react
        return;
    }

#endif /* HAS_CL */

    this->plain_seek();
    this->plain_move();
    this->notify(Issue::ProcNextDone); // Views react
}


void
Proc::plot()
{
    State& state = this->state_;
    int num = state.num_;

    // clear particles' N, L, R
    std::vector<unsigned int>& pn = state.pn_;
    std::vector<unsigned int>& pl = state.pl_;
    std::vector<unsigned int>& pr = state.pr_;
    for (int i = 0; i < num; ++i) {
        pn[i] = 0;
        pl[i] = 0;
        pr[i] = 0;
    }

    // hydrate grid units with corresponding particle indices
    // initialise the vicinity grid in non-flat format
    float width = state.width_;
    float height = state.height_;
    unsigned int scope = state.scope_;
    int cols = 1; if (scope < width)  { cols = floor(width  / scope); }
    int rows = 1; if (scope < height) { rows = floor(height / scope); }
    unsigned int grid_size = cols * rows;
    auto grid = std::vector<std::vector<int>>(grid_size); // signed!
    float unit_width = width / cols;
    float unit_height = height / rows;
    std::vector<float>& px = state.px_;
    std::vector<float>& py = state.py_;
    std::vector<int>& pgcol = state.pgcol_;
    std::vector<int>& pgrow = state.pgrow_;
    int col;
    int row;
    for (int i = 0; i < num; ++i) {
        // the last column/row may be spatially slightly bigger than the rest,
        // if the grid does not divide neatly into whole numbers
        col = floor(px[i] / unit_width);  if (col >= cols) { col = cols - 1; }
        row = floor(py[i] / unit_height); if (row >= rows) { row = rows - 1; }
        pgcol[i] = col;
        pgrow[i] = row;
        grid[cols * row + col].push_back(i);
    }

    // find the size of the largest grid unit
    unsigned int max = 0;
    unsigned int count = 0;
    for (unsigned int i = 0; i < grid_size; ++i) {
        count = grid[i].size();
        if (count > max) {
            max = count;
        }
    }

    // flatten the grid, padding with -1
    auto flat = std::vector<int>();
    for (std::vector<int> unit : grid) {
        count = 0;
        for (int p : unit) {
            flat.push_back(p);
            ++count;
        }
        while (count < max) {
            flat.push_back(-1);
            ++count;
        }
    }

    this->grid_ = flat;
    this->grid_cols_ = cols;
    this->grid_rows_ = rows;
    this->grid_stride_ = max;
}


#ifdef HAS_CL

void
Proc::seek()
{
    State& state = this->state_;
    this->cl_.seek(this->grid_, this->grid_stride_, state.num_,
                   state.px_, state.py_, state.pc_, state.ps_,
                   state.pn_, state.pl_, state.pr_,
                   state.pgcol_, state.pgrow_,
                   this->grid_cols_, this->grid_rows_,
                   state.width_, state.height_, state.scope_squared_);
}


void
Proc::move()
{
    State& state = this->state_;
    this->cl_.move(state.num_, state.px_, state.py_,
                   state.pf_, state.pc_, state.ps_,
                   state.pn_, state.pl_, state.pr_,
                   state.width_, state.height_,
                   state.alpha_, state.beta_, state.speed_);
}

#endif /* HAS_CL */


void
Proc::plain_seek()
{
    State& state = this->state_;
    std::vector<int>& grid = this->grid_;
    int cols = this->grid_cols_;
    int rows = this->grid_rows_;
    int stride = this->grid_stride_;
    std::vector<int>& pgcol = state.pgcol_;
    std::vector<int>& pgrow = state.pgrow_;

    // for each particle index
    for (int srci = 0; srci < state.num_; ++srci) {
        this->plain_seek_vicinity(grid, stride,
                                  pgcol[srci], pgrow[srci], cols, rows, srci);
    }
}


void
Proc::plain_seek_vicinity(std::vector<int>& grid, unsigned int stride,
                          int col, int row, int cols, int rows, int srci)
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
    int dsti;

    // for every unit in the vicinity (neighborhood)
    for (unsigned int v = 0; v < 54; v += 6) {
        // for each particle index within the unit
        for (unsigned int p = 0; p < stride; ++p) {
            dsti = grid[cols * (vic[v + 1] * stride) + (vic[v] * stride) + p];
            // avoid grid padding area (meaning no particle left in that unit)
            if (dsti == -1) {
                break;
            }
            // avoid redundant calculations
            if (srci <= dsti) {
                continue;
            }
            this->plain_seek_tally(srci, dsti,
                                   vic[v + 2], vic[v + 3],
                                   vic[v + 4], vic[v + 5]);
        }
    }
}


void
Proc::plain_seek_tally(unsigned int srci, unsigned int dsti,
                       bool cunder, bool cover, bool runder, bool rover)
{
    State& state = this->state_;
    float srcx = state.px_[srci];
    float srcy = state.py_[srci];
    float dstx = state.px_[dsti];
    float dsty = state.py_[dsti];
    float width = this->state_.width_;
    float height = this->state_.height_;
    float dx = dstx - srcx;
    if (cunder) { dx -= width; }  else if (cover) { dx += width; }
    float dy = dsty - srcy;
    if (runder) { dy -= height; } else if (rover) { dy += height; }

    // ignore comparisons outside the vicinity scope
    if ((dx * dx) + (dy * dy) > this->state_.scope_squared_)
    {
        return;
    }

    ++state.pn_[srci];
    ++state.pn_[dsti];
    float srcc = state.pc_[srci];
    float srcs = state.ps_[srci];
    float dstc = state.pc_[dsti];
    float dsts = state.ps_[dsti];
    if (0.0f > (dx * srcs) - (dy * srcc)) { ++state.pr_[srci]; }
    else                                  { ++state.pl_[srci]; }
    if (0.0f < (dx * dsts) - (dy * dstc)) { ++state.pr_[dsti]; }
    else                                  { ++state.pl_[dsti]; }
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
                    * Util::signum(static_cast<int>(pr[i] - pl[i]))), TAU);
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

