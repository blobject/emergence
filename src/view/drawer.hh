#pragma once

//#include "../gl/indexbuffer.hh"
#include "../gl/shader.hh"
#include "../gl/vertexarray.hh"


// Drawer: Clear and draw/render the visualisation.

class Drawer
{
 public:
  static void Clear();
  static void Draw(unsigned int size,
                   unsigned int count,
                   const VertexArray &va,
                   //const IndexBuffer &ib,
                   const Shader &shader);
};

