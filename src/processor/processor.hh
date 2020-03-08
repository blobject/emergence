#pragma once

#include "../gl/shader.hh"
#include "../gl/vertexarray.hh"
#include "../state/state.hh"


// ProcessOut: For processor.cc/Process() to return multiple values.

struct ProcessOut
{
  unsigned int Num;
  VertexArray* VertexArray;
  Shader*      Shader;
};


// Processor: Primordial particle system algorithm implementation.

class Processor
{
 public:
  State state_;

  Processor(State &state);

  static void InitCl();
  ProcessOut  Process();
};

