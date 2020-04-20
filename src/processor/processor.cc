#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "processor.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Processor::Processor(State &state)
  : state_(state)
{
  //this->state_.Register(*this);
  this->InitCl();

  // initialise grid
  auto &ps = state.particles_;
  unsigned int width = state.width_;
  unsigned int height = state.height_;
  unsigned int nradius = state.nradius_;
  unsigned int cols = 1;
  unsigned int rows = 1;
  unsigned int unit_width = width;
  unsigned int unit_height = height;
  if (nradius < width)
  {
    cols = floor(width / nradius);
    unit_width = width / cols;
  }
  if (nradius < height)
  {
    rows = floor(height / nradius);
    unit_height = height / cols;
  }
  this->grid_ = std::vector<std::vector<std::vector<unsigned int> > >
    (cols, std::vector<std::vector<unsigned int> >(rows));
}


Processor::~Processor()
{
  //this->state_.Deregister(*this);
}


ProcessOut::~ProcessOut()
{
  //delete this->vertex_array;
  //delete this->shader;
}


// InitCl: Initialise OpenCL.

/**/
void
Processor::InitCl()
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
//*/


ProcessOut
Processor::All()
{
  // recognise world state
  auto &particles = this->state_.particles_;
  unsigned int num = this->state_.num_;
  unsigned int width = this->state_.width_;
  unsigned int height = this->state_.height_;

  // initialise shape
  float radius = particles[0].radius / 2;
  float shape[] = { 0.0f,   radius,
                   -radius, 0.0f,
                    radius, 0.0f,
                    0.0f,  -radius };

  // initialise translations
  float trans[2 * num];
  int index = 0;
  for (const Particle &particle : particles)
  {
    trans[index++] = particle.x;
    trans[index++] = particle.y;
  }

  // handle gpu buffers
  this->va_ = new VertexArray();
  VertexBuffer vb_shape(shape, sizeof(shape));
  VertexBuffer vb_trans(trans, sizeof(trans));
  this->va_->AddBuffer(0, vb_shape, VertexBufferLayout::Make<float>(2));
  this->va_->AddBuffer(1, vb_trans, VertexBufferLayout::Make<float>(2));

  // instancing
  glVertexAttribDivisor(1, 1);

  // shading
  glm::mat4 projection = glm::ortho(0.0f, (float) width,
                                    0.0f, (float) height,
                                   -1.0f, 1.0f);
  auto shader = new Shader("../src/shader/emergence.shader");
  shader->Bind();
  shader->SetUniformMat4f("mvp", projection);

  // unbinding
  shader->Unbind(); // optional
  vb_shape.Unbind(); // optional
  vb_trans.Unbind(); // optional
  this->va_->Unbind();

  return { num, this->va_, shader };
}


VertexArray*
Processor::Next()
{
  this->Reset();
  this->Seek();
  this->Move();

  VertexArray* va = this->va_;
  unsigned int width = this->state_.width_;
  float trans[2 * this->state_.num_];
  int index = 0;
  for (auto &particle : this->state_.particles_)
  {
    trans[index++] = particle.x;
    trans[index++] = particle.y;
  }
  VertexBuffer vb(trans, sizeof(trans));
  va->AddBuffer(1, vb, VertexBufferLayout::Make<float>(2));
  vb.Unbind();
  va->Unbind();
  return va;
}


void
Processor::Reset()
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
Processor::Seek()
{
  auto &grid = this->grid_;
  auto &ps = this->state_.particles_;

  // calculate grid measurements
  unsigned int width = this->state_.width_;
  unsigned int height = this->state_.height_;
  unsigned int nradius = this->state_.nradius_;
  unsigned int cols = 1;
  unsigned int rows = 1;
  unsigned int unit_width = width;
  unsigned int unit_height = height;
  if (nradius < width)
  {
    cols = floor(width / nradius);
    unit_width = width / cols;
  }
  if (nradius < height)
  {
    rows = floor(height / nradius);
    unit_height = height / cols;
  }

  // hydrate grid with particle indices
  for (unsigned int i = 0; i < ps.size(); ++i)
  {
    grid[floor(ps[i].x / unit_width)][floor(ps[i].y / unit_height)].push_back(i);
  }

  // do core calculation of seek:
  // ie. update .n, .l, .r fields of particles
  // this is the naive grid- and euclidean-distance-based algorithm
  unsigned int nradsq = this->state_.nradius_squared_;
  unsigned int dx;
  unsigned int dy;
  int i;
  int j;
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
        // look at neighboring grid units
        for (int c = col - 1; c <= col + 1; ++c)
        {
          // wrap around
          i = c;
          if      (i < 0)     { i += cols; }
          else if (i >= cols) { i -= cols; }
          for (int r = row - 1; r <= row + 1; ++r)
          {
            // wrap around
            j = r;
            if      (j < 0)     { j += rows; }
            else if (j >= rows) { j -= rows; }
            // for each particle index in the neighbor
            for (unsigned int dst_num = 0; dst_num < grid[i][j].size(); ++dst_num)
            {
              dst_i = grid[i][j][dst_num];
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
              if (dx * dx + dy * dy > nradsq)
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
Processor::Move()
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
    delta = alpha + (beta * p.n * Util::Signum(p.r - p.l));
    p.phi += delta;
    if      (p.phi < 0)    { p.phi += TAU; }
    else if (p.phi >= TAU) { p.phi -= TAU; }
    p.s = sin(p.phi);
    p.c = cos(p.phi);
    p.x += speed * p.c;
    if      (p.x < 0)      { p.x += width; }
    else if (p.x >= width) { p.x -= width; }
    p.y += speed * p.s;
    if      (p.y < 0)       { p.y += height; }
    else if (p.y >= height) { p.y -= height; }
  }
}

