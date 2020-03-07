#include <GL/glew.h>

#include "vbuffer.hh"
#include "../util/util.hh"


VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
  DOGL(glGenBuffers(1, &this->id_));
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, this->id_));
  DOGL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}


VertexBuffer::~VertexBuffer()
{
  DOGL(glDeleteBuffers(1, &this->id_));
}


void
VertexBuffer::Bind() const
{
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, this->id_));
}


void
VertexBuffer::Unbind() const
{
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

