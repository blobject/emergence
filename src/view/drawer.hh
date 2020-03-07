#pragma once

#include "../gl/ibuffer.hh"
#include "../gl/shader.hh"
#include "../gl/varray.hh"


// Drawer: Clear and draw/render the visualisation.

class Drawer
{
 public:
  static void Clear();
  static void Draw(const VertexArray &va,
                   const IndexBuffer &ib,
                   const Shader &shader);
};

