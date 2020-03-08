#include <GL/glew.h>

#include "indexbuffer.hh"
#include "../util/util.hh"


IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
  : count_(count)
{
  DOGL(glGenBuffers(1, &this->id_));
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
  DOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int),
                    data, GL_STATIC_DRAW));
}


IndexBuffer::~IndexBuffer()
{
  DOGL(glDeleteBuffers(1, &this->id_));
}


void
IndexBuffer::Bind() const
{
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
}


void
IndexBuffer::Unbind() const
{
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

