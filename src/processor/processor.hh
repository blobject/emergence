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
  //unsigned int num_;
  //VertexArray* va_;
  //Shader* shader_;

 public:
  Processor(State &state);
  ~Processor();

  inline State &get_state() const { return this->state_; }

  //void        Update(Subject &next) override;
  static void InitCl();
  ProcessOut  All();
  ProcessOut  Right();
};

