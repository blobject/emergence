#include <iostream>
#include <GL/glew.h>

#include "varray.hh"
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
VertexArray::AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
  this->Bind();
  vb.Bind();
  const auto &elements = layout.get_elements();
  unsigned int offset = 0;
  for (unsigned int i = 0; i < elements.size(); ++i)
  {
    const auto &element = elements[i];
    DOGL(glEnableVertexAttribArray(i));
    DOGL(glVertexAttribPointer(i, element.count_, element.type_,
                               element.normalised_, layout.get_stride(),
                               (const void*) (long) offset));
    offset += element.count_ *
      VertexBufferLayoutElement::get_type_size(element.type_);
  }
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

