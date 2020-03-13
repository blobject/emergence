#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "processor.hh"
#include "../util/util.hh"


Processor::Processor(State &state)
  : state_(state)
{
  //this->state_.Register(*this);
  this->InitCl();
}


Processor::~Processor()
{
  //this->state_.Deregister(*this);
}


ProcessOut::~ProcessOut()
{
  delete this->vertex_array;
  delete this->shader;
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


//void
//Processor::Update(Subject &next)
//{
//  this->state_ = static_cast<State&>(next);
//  this->Process();
//}


ProcessOut
Processor::All()
{
  // recognise world state
  std::vector<Particle> particles = this->state_.particles_;
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
  auto va = new VertexArray();
  VertexBuffer vb_shape(shape, sizeof(shape));
  VertexBuffer vb_trans(trans, sizeof(trans));
  va->AddBuffer(0, vb_shape, VertexBufferLayout::Make<float>(2));
  va->AddBuffer(1, vb_trans, VertexBufferLayout::Make<float>(2));

  // instancing
  glVertexAttribDivisor(1, 1);

  // shading
  glm::mat4 projection = glm::ortho(0.0f, (float) width,
                                    0.0f, (float) height,
                                   -1.0f, 1.0f);
  auto shader = new Shader("../etc/basic.glsl");
  shader->Bind();
  shader->SetUniformMat4f("mvp", projection);

  // unbinding
  shader->Unbind(); // optional
  vb_shape.Unbind(); // optional
  vb_trans.Unbind(); // optional
  va->Unbind();

  return { num, va, shader };
}


ProcessOut
Processor::Right()
{
  // recognise world state
  std::vector<Particle> particles = this->state_.particles_;
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
  for (auto &particle : particles)
  {
    particle.x += 2;
  }
  float trans[2 * num];
  int index = 0;
  for (const Particle &particle : particles)
  {
    trans[index++] = particle.x;
    trans[index++] = particle.y;
  }

  // handle gpu buffers
  auto va = new VertexArray();
  VertexBuffer vb_shape(shape, sizeof(shape));
  VertexBuffer vb_trans(trans, sizeof(trans));
  va->AddBuffer(0, vb_shape, VertexBufferLayout::Make<float>(2));
  va->AddBuffer(1, vb_trans, VertexBufferLayout::Make<float>(2));

  // instancing
  glVertexAttribDivisor(1, 1);

  // shading
  glm::mat4 projection = glm::ortho(0.0f, (float) width,
                                    0.0f, (float) height,
                                   -1.0f, 1.0f);
  auto shader = new Shader("../etc/basic.glsl");
  shader->Bind();
  shader->SetUniformMat4f("mvp", projection);

  // unbinding
  shader->Unbind(); // optional
  vb_shape.Unbind(); // optional
  vb_trans.Unbind(); // optional
  va->Unbind();

  return { num, va, shader };
}

