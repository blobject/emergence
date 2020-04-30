#include <GL/glew.h>

#include "vertexarray.hh"
#include "../util/util.hh"


VertexArray::VertexArray()
{
  DOGL(glGenVertexArrays(1, &this->id_));
}


VertexArray::~VertexArray()
{
  DOGL(glDeleteVertexArrays(1, &this->id_));
}


void
VertexArray::AddBuffer(unsigned int id, const VertexBuffer &vb,
                       const LayoutItem &item)
{
  this->Bind();
  vb.Bind();
  vb.Buffer();
  DOGL(glEnableVertexAttribArray(id));
  DOGL(glVertexAttribPointer(id, item.count, item.type, item.normalised,
                             item.stride, static_cast<const void*>(0)));
}


void
VertexArray::Bind() const
{
  DOGL(glBindVertexArray(this->id_));
}


void
VertexArray::Unbind() const
{
  DOGL(glBindVertexArray(0));
}

