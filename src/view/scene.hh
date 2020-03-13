#pragma once

//#include "../gl/indexbuffer.hh"
#include "../gl/shader.hh"
#include "../gl/vertexarray.hh"


// Scene: Renderer of the visualisation.

class Scene
{
 public:
  static void Clear();
  static void Draw(unsigned int size,
                   unsigned int count,
                   const VertexArray &va,
                   //const IndexBuffer &ib,
                   const Shader &shader);
};

