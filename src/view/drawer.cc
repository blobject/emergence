#include <GL/glew.h>

#include "drawer.hh"
#include "../util/util.hh"


void
Drawer::Clear()
{
  DOGL(glClear(GL_COLOR_BUFFER_BIT));
}


void
Drawer::Draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader)
{
  shader.Bind();
  va.Bind();
  ib.Bind();
  DOGL(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));
}

