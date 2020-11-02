#include <GL/glew.h>
#include <thread>

#include "sys.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Sys::Sys(State &state)
  : state_(state)
{
  this->InitCl();
  this->InitGrid();
}


// InitCl: Initialise OpenCL

void
Sys::InitCl()
{
  std::vector<cl::Platform> platforms;
  cl::Platform default_platform;
  std::vector<cl::Device> devices;
  cl::Device default_device;

  cl::Platform::get(&platforms);
  if (0 == platforms.size())
  {
    Util::Err("no platforms found.");
    return;
  }
  this->cl_platform_ = platforms[0];
  for (auto &platform : platforms)
  {
    if ("FULL_PROFILE" != platform.getInfo<CL_PLATFORM_PROFILE>())
    {
      continue;
    }
    this->cl_platform_ = platform;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (0 == devices.size())
    {
      continue;
    }
    cl_device_ = devices[0];
    break;
  }
  // TODO:
  // check if cl_device_ not set
  Util::Out("opencl platform: "
            + this->cl_platform_.getInfo<CL_PLATFORM_NAME>());
  Util::Out("opencl device: "
            + this->cl_device_.getInfo<CL_DEVICE_NAME>());
}


// Reset: Make the neighborhood grid

void
Sys::InitGrid()
{
  auto &ps = this->state_.particles_;
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
  this->grid_ = Grid (cols, std::vector<std::vector<unsigned int> >(rows));
}


// Next: Make the particle system act one iteration

void
Sys::Next()
{
  this->Reset();
  this->Seek();
  this->Move();
}


// Reset: Zero out particles' neighborhood count and remake the grid

void
Sys::Reset()
{
  auto &ps = this->state_.particles_;
  auto &grid = this->grid_;
  for (auto &p : ps)
  {
    p.n = 0;
    p.l = 0;
    p.r = 0;
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
  for (unsigned int i = 0; i < ps.size(); ++i)
  {
    col = floor(ps[i].x / unit_width);
    row = floor(ps[i].y / unit_height);
    if (col >= cols) { col -= cols; }
    if (row >= rows) { row -= rows; }
    grid[col][row].push_back(i);
  }
}


// Seek: For each particle calculate new N, L, R.

void
Sys::Seek()
{
  auto &ps = this->state_.particles_;
  auto &grid = this->grid_;
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();

  this->SeekFrom(ps, grid, cols, rows);
}


// SeekFrom: Seek() helper, loop through each particle.

void
Sys::SeekFrom(std::vector<Particle> &ps, Grid &grid,
              unsigned int cols, unsigned int rows)
{
  for (unsigned int col = 0; col < cols; ++col)
  {
    for (unsigned int row = 0; row < rows; ++row)
    {
      // for each particle index
      for (unsigned int p = 0; p < grid[col][row].size(); ++p)
      {
        this->SeekTo(ps, grid, col, row, cols, rows, grid[col][row][p]);
      }
    }
  }
}


// SeekTo: Seek() helper, loop through every other particle in the grid
//         neighborhood.

void
Sys::SeekTo(std::vector<Particle> &ps, Grid &grid,
            unsigned int col,  unsigned int row,
            unsigned int cols, unsigned int rows,
            unsigned int srci)
{
  unsigned int ncols[3];
  unsigned int nrows[3];

  // neighboring columns
  ncols[0] = col - 1; ncols[1] = col; ncols[2] = col + 1;
  if      (col == 0)        { ncols[0] += cols; }
  else if (col == cols - 1) { ncols[2] -= cols; }
  for (unsigned int c : ncols)
  {
    // neighboring rows
    nrows[0] = row - 1; nrows[1] = row; nrows[2] = row + 1;
    if      (row == 0)        { nrows[0] += rows; }
    else if (row == rows - 1) { nrows[2] -= rows; }
    for (unsigned int r : nrows)
    {
      // for each particle index in the neighbor unit
      std::vector<unsigned int> &unit = grid[c][r];
      for (unsigned int p = 0; p < unit.size(); ++p)
      {
        this->SeekTally(ps, srci, unit[p]);
      }
    }
  }
}


// SeekTally: Seek() helper that updates N, L, R given the two particles given
//            respectively by SeekFrom() and SeekTo()

void
Sys::SeekTally(std::vector<Particle> &ps,
               unsigned int srci, unsigned int dsti)
{
  // avoid redundant calculations
  if (srci <= dsti)
  {
    return;
  }

  Particle &src = ps[srci];
  Particle &dst = ps[dsti];
  int dx = dst.x - src.x;
  int dy = dst.y - src.y;

  // ignore comparisons outside the neighborhood radius (approx. scope)
  if ((dx * dx) + (dy * dy) > this->state_.scope_squared_)
  {
    return;
  }

  int w = this->state_.half_width_;
  int h = this->state_.half_height_;
  dx = Util::ModI(dx + w, this->state_.width_) - w;
  dy = Util::ModI(dy + h, this->state_.height_) - h;
  ++src.n;
  ++dst.n;
  if (0.0f > (dx * src.s) - (dy * src.c)) { ++src.r; }
  else                                    { ++src.l; }
  if (0.0f < (dx * dst.s) - (dy * dst.c)) { ++dst.r; }
  else                                    { ++dst.l; }
}


// Move: Update particles' location and direction

void
Sys::Move()
{
  //std::cout << this->state_.particles_[0].s << ", " << this->state_.particles_[0].c << std::endl;
  for (auto &p : this->state_.particles_)
  {
    p.phi = Util::ModF(p.phi + this->state_.alpha_
                       + (this->state_.beta_ * p.n
                          * Util::Signum(static_cast<int>(p.r - p.l))), TAU);
    p.s = sinf(p.phi);
    p.c = cosf(p.phi);
    p.x = Util::ModI(p.x + static_cast<int>(this->state_.speed_ * p.c),
                     this->state_.width_);
    p.y = Util::ModI(p.y + static_cast<int>(this->state_.speed_ * p.s),
                     this->state_.height_);
  }
}

