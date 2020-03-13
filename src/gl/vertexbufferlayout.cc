#include <GL/glew.h>

#include "vertexbufferlayout.hh"


// TypeSize: Convenience function that converts OpenGL data
//           constants to corresponding data sizes.

unsigned int
LayoutItem::TypeSize(unsigned int type)
{
  switch (type)
  {
    case GL_FLOAT:         return 4;
    case GL_UNSIGNED_INT:  return 4;
    case GL_UNSIGNED_BYTE: return 1;
  }
  return 0;
}


template<typename T> LayoutItem
VertexBufferLayout::Make(unsigned int count)
{
  //static_assert(false);
}

template<> LayoutItem
VertexBufferLayout::Make<float>(unsigned int count)
{
  return { GL_FLOAT, count, GL_FALSE,
           count * LayoutItem::TypeSize(GL_FLOAT) };
}

template<> LayoutItem
VertexBufferLayout::Make<unsigned int>(unsigned int count)
{
  return { GL_UNSIGNED_INT, count, GL_FALSE,
           count * LayoutItem::TypeSize(GL_UNSIGNED_INT) };
}

template<> LayoutItem
VertexBufferLayout::Make<unsigned char>(unsigned int count)
{
  return { GL_UNSIGNED_BYTE, count, GL_TRUE,
           count * LayoutItem::TypeSize(GL_UNSIGNED_BYTE) };
}

