#pragma once

#include <vector>


// VertexBufferLayoutElement: Helper for OpenGL vertex arrays.

struct VertexBufferLayoutElement
{
  unsigned int  type_;
  unsigned int  count_;
  unsigned char normalised_;

  static unsigned int get_type_size(unsigned int type);
};


// VertexBufferLayout: Helper for OpenGL vertex arrays.

class VertexBufferLayout
{
 private:
  std::vector<VertexBufferLayoutElement> elements_;
  unsigned int                           stride_;

 public:
  VertexBufferLayout();

  inline const std::vector<VertexBufferLayoutElement>
  get_elements() const { return this->elements_; }
  inline unsigned int       get_stride() const { return this->stride_; }
  template<typename T> void Push(unsigned int count);
  template<> void           Push<float>(unsigned int count);
  template<> void           Push<unsigned int>(unsigned int count);
  template<> void           Push<unsigned char>(unsigned int count);
};

