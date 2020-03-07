#pragma once


// IndexBuffer: Wrapper around OpenGL index buffers.

class IndexBuffer
{
 private:
  unsigned int id_;
  unsigned int count_;

 public:
  IndexBuffer(const unsigned int* data, unsigned int count);
  ~IndexBuffer();

  inline unsigned int get_count() const { return this->count_; }
  void                Bind() const;
  void                Unbind() const;
};

