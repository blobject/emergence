#include <GL/glew.h>

#include "sys.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Sys::Sys(State &state)
  : state_(state)
{
  this->cl_ = new Cl();
  this->cl_good_ = this->cl_->Good();
  if (! this->cl_good_)
  {
    Util::Out("Proceeding without OpenCL parallelisation.");
  }
  this->GenGrid();
}


// GenGrid: Make the neighborhood grid.

void
Sys::GenGrid()
{
  unsigned int width = this->state_.width_;
  unsigned int height = this->state_.height_;
  unsigned int scope = this->state_.scope_;
  unsigned int cols = 1;
  unsigned int rows = 1;
  float unit_width = static_cast<float>(width);
  float unit_height = static_cast<float>(height);
  if (scope < width)
  {
    cols = floor(width / scope);
    unit_width = width / static_cast<float>(cols);
  }
  if (scope < height)
  {
    rows = floor(height / scope);
    unit_height = height / static_cast<float>(cols);
  }
  this->grid_ = Grid(cols, std::vector<std::vector<unsigned int> >(rows));
}


// Next: Let the particle system perform one action step.

void
Sys::Next()
{
  this->Reset();
  if (this->cl_good_)
  {
    this->Seek();
    this->Move();
    return;
  }
  this->PlainSeek();
  this->PlainMove();
}


// Seek: Calculate new N, L, R for each particle.

void
Sys::Seek()
{
  this->PlainSeek();
  this->cl_->Seek();
}


// Move: Update position and direction of particles.

void
Sys::Move()
{
  State &state = this->state_;
  //this->PlainMove();
  this->cl_->Move(state.num_, state.px_, state.py_,
                  state.pf_, state.ps_, state.pc_,
                  state.pn_, state.pl_, state.pr_,
                  state.width_, state.height_,
                  state.alpha_, state.beta_, state.speed_);
}


// Reset: Zero out particles' neighborhood count and remake the grid.

void
Sys::Reset()
{
  State &state = this->state_;
  std::vector<float> &px = state.px_;
  std::vector<float> &py = state.py_;
  std::vector<unsigned int> &pn = state.pn_;
  std::vector<unsigned int> &pl = state.pl_;
  std::vector<unsigned int> &pr = state.pr_;
  unsigned int num = state.num_;
  Grid &grid = this->grid_;
  for (int i = 0; i < num; ++i)
  {
    pn[i] = 0;
    pl[i] = 0;
    pr[i] = 0;
  }
  for (auto &col : grid)
  {
    for (auto &row : col)
    {
      row.clear();
    }
  }
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();
  unsigned int col;
  unsigned int row;
  float unit_width = this->state_.width_ / static_cast<float>(cols);
  float unit_height = this->state_.height_ / static_cast<float>(rows);
  for (unsigned int i = 0; i < num; ++i)
  {
    col = floor(px[i] / unit_width);
    row = floor(py[i] / unit_height);
    if (col >= cols) { col -= cols; }
    if (row >= rows) { row -= rows; }
    grid[col][row].push_back(i);
  }
}


// PlainSeek: Non-CL version of Seek.

void
Sys::PlainSeek()
{
  Grid &grid = this->grid_;
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();

  this->SeekFrom(grid, cols, rows);
}


// SeekFrom: PlainSeek() helper, loop through each particle.

void
Sys::SeekFrom(Grid &grid, unsigned int cols, unsigned int rows)
{
  for (unsigned int col = 0; col < cols; ++col)
  {
    for (unsigned int row = 0; row < rows; ++row)
    {
      // for each particle index
      for (unsigned int p = 0; p < grid[col][row].size(); ++p)
      {
        this->SeekTo(grid, col, row, cols, rows, grid[col][row][p]);
      }
    }
  }
}


// SeekTo: PlainSeek() helper, loop through every other particle in the grid
//         neighborhood.

void
Sys::SeekTo(Grid &grid,
            unsigned int col,  unsigned int row,
            unsigned int cols, unsigned int rows,
            unsigned int srci)
{
  bool c_under;
  bool c_over;
  bool r_under;
  bool r_over;

  // neighboring columns
  for (int c : {col - 1, col, col + 1})
  {
    c_under = false;
    c_over = false;
    if      (c < 0)     { c += cols; c_under = true; }
    else if (c >= cols) { c -= cols; c_over = true; }

    // neighboring rows
    for (int r : {row - 1, row, row + 1})
    {
      r_under = false;
      r_over = false;
      if      (r < 0)     { r += rows; r_under = true; }
      else if (r >= rows) { r -= rows; r_over = true; }

      // for each particle index in the neighbor unit
      std::vector<unsigned int> &unit = grid[c][r];
      for (unsigned int p : unit)
      {
        this->SeekTally(srci, p, c_under, c_over, r_under, r_over);
      }
    }
  }
}


// SeekTally: PlainSeek() helper that updates N, L, R given the two particles
//            given respectively by SeekFrom() and SeekTo().

void
Sys::SeekTally(unsigned int srci, unsigned int dsti,
               bool c_under, bool c_over, bool r_under, bool r_over)
{
  // avoid redundant calculations
  if (srci <= dsti)
  {
    return;
  }

  State &state = this->state_;
  float srcx = state.px_[srci];
  float srcy = state.py_[srci];
  float dstx = state.px_[dsti];
  float dsty = state.py_[dsti];
  int width = this->state_.width_;
  int height = this->state_.height_;
  int dx = dstx - srcx;
  if      (c_under) { dx = -1 * Util::ModI(srcx - dstx, width); }
  else if (c_over)  { dx = Util::ModI(dx, width); }
  int dy = dsty - srcy;
  if      (r_under) { dy = -1 * Util::ModI(srcy - dsty, height); }
  else if (r_over)  { dy = Util::ModI(dy, height); }

  // ignore comparisons outside the neighborhood radius (approx. scope)
  if ((dx * dx) + (dy * dy) > this->state_.scope_squared_)
  {
    return;
  }

  ++state.pn_[srci];
  ++state.pn_[dsti];
  float srcs = state.ps_[srci];
  float srcc = state.pc_[srci];
  float dsts = state.ps_[dsti];
  float dstc = state.pc_[dsti];
  if (0.0f > (dx * srcs) - (dy * srcc)) { ++state.pr_[srci]; }
  else                                    { ++state.pl_[srci]; }
  if (0.0f < (dx * dsts) - (dy * dstc)) { ++state.pr_[dsti]; }
  else                                    { ++state.pl_[dsti]; }
}


// PlainMove: Non-CL version of Move.

void
Sys::PlainMove()
{
  State &state = this->state_;
  float width = state.width_;
  float height = state.height_;
  float alpha = state.alpha_;
  float beta = state.beta_;
  float speed = state.speed_;
  std::vector<float> &px = state.px_;
  std::vector<float> &py = state.py_;
  std::vector<float> &pf = state.pf_;
  std::vector<float> &ps = state.ps_;
  std::vector<float> &pc = state.pc_;
  std::vector<unsigned int> &pn = state.pn_;
  std::vector<unsigned int> &pl = state.pl_;
  std::vector<unsigned int> &pr = state.pr_;

  for (int i = 0; i < state.num_; ++i)
  {
    pf[i] = Util::ModF(pf[i] + alpha
                       + (beta * pn[i]
                          * Util::Signum(static_cast<int>(pr[i] - pl[i]))),
                       TAU);
    ps[i] = sinf(pf[i]);
    pc[i] = cosf(pf[i]);
    px[i] = Util::ModF(px[i] + speed * pc[i], width);
    py[i] = Util::ModF(py[i] + speed * ps[i], height);
  }
}

