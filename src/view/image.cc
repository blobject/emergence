#include "image.hh"
#include <GL/gl.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <vector>


bool
Image::out(const std::string& path, int width, int height)
{
  GLsizei chans = 3;
  GLsizei stride = chans * width;
  stride += (stride % 4) ? (4 - stride % 4) : 0;
  auto buffer = std::vector<char>(stride * height);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
  stbi_flip_vertically_on_write(true);
  return stbi_write_png(path.c_str(), width, height, chans, buffer.data(),
                        stride);
}

