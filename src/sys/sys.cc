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


// InitCl: Initialise OpenCL.

void
Sys::InitCl()
{
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;

  cl::Platform::get(&platforms);
  if (0 == platforms.size())
  {
    Util::Err("no cl platforms found.");
    return;
  }
  this->cl_platform_ = platforms.front();
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
    this->cl_device_ = devices.front();
    break;
  }
  if (this->cl_device_.getInfo<CL_DEVICE_NAME>().empty())
  {
    Util::Err("no cl devices found.");
    return;
  }

  this->ExecCl();
}


// Reset: Make the neighborhood grid.

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


// ExecCl: Compute using the GPU.

void
Sys::ExecCl()
{
  /**
  std::string source =
    "__kernel void\n"
    "foo(\n"
    "  uint n,\n"
    "  __global const float* a,\n"
    "  __global const float* b,\n"
    "  __global float* c)\n"
    "{\n"
    "  int i = get_global_id(0);\n"
    "  if (i < n)\n"
    "  {\n"
    "    c[i] = a[i] + b[i];\n"
    "  }\n"
    "}\n";
  const int N = 1 << 4;
  try
  {
    cl::Context context(this->cl_device_);
    cl::CommandQueue queue(context, this->cl_device_);
    cl::Program program(context, source, CL_TRUE);
    cl::Kernel foo(program, "foo");
    std::vector<float> a(N, 12);
    std::vector<float> b(N, 30);
    std::vector<float> c(N);
    cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                a.size() * sizeof(float), a.data());
    cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                b.size() * sizeof(float), b.data());
    cl::Buffer C(context, CL_MEM_READ_WRITE, c.size() * sizeof(float));
    foo.setArg(0, static_cast<cl_int>(N));
    foo.setArg(1, A);
    foo.setArg(2, B);
    foo.setArg(3, C);
    queue.enqueueNDRangeKernel(foo, cl::NullRange, N, cl::NullRange);
    queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(float), c.data());
    for (int i = 0; i < c.size(); i++)
    {
      std::cout << c[i] << ", ";
    }
    std::cout << std::endl;
  }
  catch (cl_int err) {
    std::cout << "Exception\n";
    std::cerr << "ERROR: " << err << std::endl;
  }
  //*/
}


void
Sys::ClSeek()
{
}


// Next: Let the particle system perform one action step.

void
Sys::Next()
{
  this->Reset();
  this->Seek();
  this->Move();
}


// Reset: Zero out particles' neighborhood count and remake the grid.

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
  //this->ClSeek();
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
        this->SeekTally(ps, srci, p, c_under, c_over, r_under, r_over);
      }
    }
  }
}


// SeekTally: Seek() helper that updates N, L, R given the two particles given
//            respectively by SeekFrom() and SeekTo().

void
Sys::SeekTally(std::vector<Particle> &ps,
               unsigned int srci, unsigned int dsti,
               bool c_under, bool c_over, bool r_under, bool r_over)
{
  // avoid redundant calculations
  if (srci <= dsti)
  {
    return;
  }

  Particle &src = ps[srci];
  Particle &dst = ps[dsti];
  int width = this->state_.width_;
  int height = this->state_.height_;
  int dx = dst.x - src.x;
  if      (c_under) { dx = -1 * Util::ModI(src.x - dst.x, width); }
  else if (c_over)  { dx = Util::ModI(dx, width); }
  int dy = dst.y - src.y;
  if      (r_under) { dy = -1 * Util::ModI(src.y - dst.y, height); }
  else if (r_over)  { dy = Util::ModI(dy, height); }

  // ignore comparisons outside the neighborhood radius (approx. scope)
  if ((dx * dx) + (dy * dy) > this->state_.scope_squared_)
  {
    return;
  }

  ++src.n;
  ++dst.n;
  if (0.0f > (dx * src.s) - (dy * src.c)) { ++src.r; }
  else                                    { ++src.l; }
  if (0.0f < (dx * dst.s) - (dy * dst.c)) { ++dst.r; }
  else                                    { ++dst.l; }
}


// Move: Update particles' location and direction.

void
Sys::Move()
{
  float width = this->state_.width_;
  float height = this->state_.height_;
  float alpha = this->state_.alpha_;
  float beta = this->state_.beta_;
  float speed = this->state_.speed_;

  for (auto &p : this->state_.particles_)
  {
    p.phi = Util::ModF(p.phi + alpha
                       + (beta * p.n
                          * Util::Signum(static_cast<int>(p.r - p.l))), TAU);
    p.s = sinf(p.phi);
    p.c = cosf(p.phi);
    p.x = Util::ModI(p.x + static_cast<int>(speed * p.c), width);
    p.y = Util::ModI(p.y + static_cast<int>(speed * p.s), height);
  }
}

