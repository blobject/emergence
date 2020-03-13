#pragma once

#include <vector>


// LayoutItem: Helper for OpenGL vertex arrays.

struct LayoutItem
{
  unsigned int  type;
  unsigned int  count;
  unsigned char normalised;
  unsigned int  stride;

  static unsigned int TypeSize(unsigned int type);
};


// VertexBufferLayout: Helper for OpenGL vertex arrays.

class VertexBufferLayout
{
 public:
  template<typename T> static LayoutItem Make(unsigned int count);
  template<> LayoutItem Make<float>(unsigned int count);
  template<> LayoutItem Make<unsigned int>(unsigned int count);
  template<> LayoutItem Make<unsigned char>(unsigned int count);
};

