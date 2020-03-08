#pragma once


// VertexBuffer: Wrapper around OpenGL vertex buffers.

class VertexBuffer
{
 private:
  unsigned int id_;
  const void*  data_;
  unsigned int size_;

 public:
  VertexBuffer(const void* data, unsigned int size);
  ~VertexBuffer();

  void Bind() const;
  void Buffer() const;
  void Unbind() const;
};

