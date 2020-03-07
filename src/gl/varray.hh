#pragma once

#include "vbuffer.hh"
#include "vbufferlayout.hh"


// VertexArray: Wrapper around OpenGL vertex arrays.

class VertexArray
{
 private:
  unsigned int id_;

 public:
  VertexArray();
  ~VertexArray();

  void AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
  void Bind() const;
  void Unbind() const;
};

