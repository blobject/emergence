#pragma once

#include <vector>

#include "vertexbuffer.hh"
#include "vertexbufferlayout.hh"


// VertexArray: Wrapper around OpenGL vertex arrays.

class VertexArray
{
 private:
  unsigned int id_;

 public:
  VertexArray();
  ~VertexArray();

  inline unsigned int get_id() const { return this->id_; }
  void AddBuffer(unsigned int id, const VertexBuffer &vb,
                 const LayoutItem &layout);
  void Bind() const;
  void Unbind() const;
};

