#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <GL/glew.h>
#include <math.h>

#include <CL/cl2.hpp>

#include "sys.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Sys::Sys(State &state)
  : state_(state)
{
  //this->state_.Register(*this);
  this->InitCl();

  // initialise grid
  this->InitGrid();
}


//Sys::~Sys()
//{
//  this->state_.Deregister(*this);
//}


// InitCl: Initialise OpenCL.

void
Sys::InitCl()
{
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;
  cl::Platform::get(&platforms);
  Util::Out("");
  for (const auto &platform : platforms)
  {
    Util::Out("cl platform: " + platform.getInfo<CL_PLATFORM_NAME>()
              + "\n\tversion " + platform.getInfo<CL_PLATFORM_VERSION>());
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    for (const auto &device : devices)
    {
      Util::Out("cl device: " + device.getInfo<CL_DEVICE_NAME>()
                + "\n\tversion " + device.getInfo<CL_DEVICE_VERSION>());
    }
  }
  Util::Out("");
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
  this->Seek();
  this->Move();
}


void
Sys::Reset()
{
  auto &ps = this->state_.particles_;
  for (auto &p : ps)
  {
    p.n = 0;
    p.l = 0;
    p.r = 0;
  }
  auto &grid = this->grid_;
  for (auto &col : grid)
  {
    for (auto &row : col)
    {
      row.clear();
    }
  }
}


void
Sys::Regrid()
{
  auto &grid = this->grid_;
  auto &ps = this->state_.particles_;
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();
  float unit_width = this->state_.width_ / static_cast<float>(cols);
  float unit_height = this->state_.height_ / static_cast<float>(rows);
  for (unsigned int i = 0; i < ps.size(); ++i)
  {
    grid[floor(ps[i].x / unit_width)][floor(ps[i].y / unit_height)].push_back(i);
  }
}


void
Sys::Seek()
{
  this->Regrid();
  auto &grid = this->grid_;
  unsigned int cols = grid.size();
  unsigned int rows = grid[0].size();
  auto &ps = this->state_.particles_;

  // do core calculation of seek:
  // ie. update .n, .l, .r fields of particles
  // this is the naive grid- and euclidean-distance-based algorithm
  unsigned int sqscope = this->state_.scope_squared_;
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
              // avoid redundant calculation
              if (src_i <= dst_i)
              {
                continue;
              }
              // update .n, .l, .r
              Particle &src = ps[src_i];
              Particle &dst = ps[dst_i];
              dx = dst.x - src.x;
              dy = dst.y - src.y;
              if (dx * dx + dy * dy > sqscope)
              {
                continue;
              }
              ++src.n;
              ++dst.n;
              if (dx < 0) { dx *= -1; }
              if (dy < 0) { dy *= -1; }
              // "dst" is to the right of "src"
              if (0 > dx * src.s - dy * src.c) { ++src.r; } else { ++src.l; }
              // "src" is to the right of "dst"
              if (0 < dx * dst.s - dy * dst.c) { ++dst.r; } else { ++dst.l; }
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

  // do core calculation of move (ie. update .x, .y, .phi)
  for (auto &p : this->state_.particles_)
  {
    delta = alpha + (p.n * beta * (p.r - p.l > 0 ? 1 : -1));
    p.phi += delta;
    if      (p.phi < 0)    { p.phi += TAU; }
    else if (p.phi >= TAU) { p.phi -= TAU; }
    p.s = sin(p.phi);
    p.c = cos(p.phi);
    p.x += speed * p.c;
    p.y += speed * p.s;
    if (p.x < 0)       { p.x += width; }
    if (p.x >= width)  { p.x -= width; } // "else if" can result in width!
    if (p.y < 0)       { p.y += height; }
    if (p.y >= height) { p.y -= height; } // "else if" can result in height!
  }
}

