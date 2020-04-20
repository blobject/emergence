#pragma once

#include "../gl/shader.hh"
#include "../gl/vertexarray.hh"
#include "../state/state.hh"


// ProcessOut: For processor.cc/Process() to return multiple values.

struct ProcessOut
{
  unsigned int num;
  VertexArray* vertex_array;
  Shader*      shader;

  ~ProcessOut();
};


// Processor: Primordial particle system algorithm implementation.

class Processor //: public Observer
{
 private:
  State &state_;
  VertexArray* va_;
  std::vector<std::vector<std::vector<unsigned int> > > grid_;

  void Reset();
  void Seek();
  void Move();

 public:
  Processor(State &state);
  ~Processor();

  inline State &get_state() const { return this->state_; }

  static void  InitCl();
  ProcessOut   All();
  VertexArray* Next();
};

