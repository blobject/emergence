#include <GL/glew.h>

#include "vertexbuffer.hh"
#include "../util/util.hh"


VertexBuffer::VertexBuffer(const void* data, unsigned int size)
  : data_(data), size_(size)
{
  DOGL(glGenBuffers(1, &this->id_));
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
VertexBuffer::Buffer() const
{
  DOGL(glBufferData(GL_ARRAY_BUFFER, this->size_, this->data_, GL_STATIC_DRAW));
}


void
VertexBuffer::Update(const void* data)
{
  this->data_ = data;
  //this->size_ = sizeof(data);
  this->Bind();
  this->Buffer();
}


void
VertexBuffer::Unbind() const
{
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

