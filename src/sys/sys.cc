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
  this->grid_ = std::vector<std::vector<std::vector<unsigned int> > >
    (cols, std::vector<std::vector<unsigned int> >(rows));
}


void
Sys::Next()
{
  this->Reset();
  this->Mirror();
  this->Seek();
  this->Move();
}


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


// Mirror: Make a copy of particles in its current state.

void
Sys::Mirror()
{
  this->old_particles_ = std::vector<Particle>(this->state_.particles_);
}


// Seek: For each particle calculate new N, L, R.

void
Sys::Seek()
{
  auto &grid = this->grid_;
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();
  auto &os = this->old_particles_;
  auto &ps = this->state_.particles_;
  auto width = this->state_.width_;
  auto height = this->state_.height_;
  auto half_width = this->state_.half_width_;
  auto half_height = this->state_.half_height_;

  // do core calculation of seek:
  // ie. update .n, .l, .r fields of particles
  // this is the naive grid- and euclidean-distance-based algorithm
  float scope = this->state_.scope_;
  float sqscope = this->state_.scope_squared_;
  unsigned int dx;
  unsigned int dy;
  unsigned int ncols[3];
  unsigned int nrows[3];
  unsigned int src_i;
  unsigned int dst_i;
  for (unsigned int col = 0; col < cols; ++col)
  {
    for (unsigned int row = 0; row < rows; ++row)
    {
      // for each particle index
      for (unsigned int src_num = 0; src_num < grid[col][row].size(); ++src_num)
      {
        src_i = grid[col][row][src_num];
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
            for (unsigned int dst_num = 0; dst_num < unit.size(); ++dst_num)
            {
              dst_i = unit[dst_num];
              // avoid redundant calculations
              if (src_i <= dst_i)
              {
                continue;
              }
              // update .n, .l, .r
              Particle &old_src = os[src_i];
              Particle &old_dst = os[dst_i];
              Particle &new_src = ps[src_i];
              Particle &new_dst = ps[dst_i];
              dx = old_dst.x - old_src.x;
              if (dx < -scope)
                dx = fmod(old_dst.x - old_src.x + half_width, width) + half_width;
              else if (dx > scope)
                dx = fmod(old_dst.x - old_src.x + half_width, width) - half_width;
              dy = old_dst.y - old_src.y;
              if (dy < -scope)
                dy = fmod(old_dst.y - old_src.y + half_height, height) + half_height;
              else if (dy > scope)
                dy = fmod(old_dst.y - old_src.y + half_height, height) - half_height;
              // ignore comparisons outside the neighborhood radius
              if (dx * dx + dy * dy > sqscope)
              {
                continue;
              }
              ++new_src.n;
              ++new_dst.n;
              // "dst" is to the right of "src"
              if (0 > dx * old_src.s - dy * old_src.c) { ++new_src.r; }
              else                                     { ++new_src.l; }
              // "src" is to the right of "dst"
              if (0 < dx * old_dst.s - dy * old_dst.c) { ++new_dst.r; }
              else                                     { ++new_dst.l; }
            }
          }
        }
      }
    }
  }
}


void
Sys::Move()
{
  unsigned int width = this->state_.width_;
  unsigned int height = this->state_.height_;
  float speed = this->state_.speed_;
  float alpha = this->state_.alpha_;
  float beta = this->state_.beta_;
  float delta;
  float phi_rad;

  // do core calculation of move (ie. update .x, .y, .phi)
  for (auto &p : this->state_.particles_)
  {
    p.phi += alpha + (beta * p.n * Util::Signum(static_cast<int>(p.r - p.l)));
    if (p.phi < 0.0f)    { p.phi += 360.0f; }
    if (p.phi >= 360.0f) { p.phi -= 360.0f; }
    phi_rad = Util::DegToRad(p.phi);
    p.s = sinf(phi_rad);
    p.c = cosf(phi_rad);
    p.x += speed * p.c;
    p.y += speed * p.s;
    if (p.x < 0)       { p.x += width; }
    if (p.x >= width)  { p.x -= width; } // "else if" can result in == width!
    if (p.y < 0)       { p.y += height; }
    if (p.y >= height) { p.y -= height; } // "else if" can result in == height!
  }
}

