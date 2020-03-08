#include <GL/glew.h>

#include "drawer.hh"
#include "../util/util.hh"


void
Drawer::Clear()
{
  DOGL(glClear(GL_COLOR_BUFFER_BIT));
}


void
Drawer::Draw(unsigned int size, unsigned int count,
             const VertexArray &va, const Shader &shader)
{
  shader.Bind();
  va.Bind();
  //ib.Bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, size, count));
  //DOGL(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));
}

