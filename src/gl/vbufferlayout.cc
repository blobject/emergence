#include <vector>
#include <GL/glew.h>

#include "vbufferlayout.hh"


// VertexBufferLayoutElement: Convenience function that converts OpenGL data
//                            constants to corresponding data sizes.

unsigned int
VertexBufferLayoutElement::get_type_size(unsigned int type)
{
  switch (type)
  {
  case GL_FLOAT:         return 4;
  case GL_UNSIGNED_INT:  return 4;
  case GL_UNSIGNED_BYTE: return 1;
  }
  return 0;
}


VertexBufferLayout::VertexBufferLayout() : stride_(0) {}


template<typename T> void
VertexBufferLayout::Push(unsigned int count)
{
  //static_assert(false);
}


template<> void
VertexBufferLayout::Push<float>(unsigned int count)
{
  (this->elements_).push_back({ GL_FLOAT, count, GL_FALSE });
  this->stride_ += count *
    VertexBufferLayoutElement::get_type_size(GL_FLOAT);
}


template<> void
VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
  (this->elements_).push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
  this->stride_ += count *
    VertexBufferLayoutElement::get_type_size(GL_UNSIGNED_INT);
}


template<> void
VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
  (this->elements_).push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
  this->stride_ += count *
    VertexBufferLayoutElement::get_type_size(GL_UNSIGNED_BYTE);
}

