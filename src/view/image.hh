//===-- view/image.hh - image functions ------------------------*- C++ -*-===//
///
/// \file
/// Declarations of static image functions.
///
//===---------------------------------------------------------------------===//

#pragma once

#include <string>


class Image
{
 public:
  /// out(): Export OpenGL window to a png image file.
  /// \param path  path to destination png file
  /// \param width  OpenGL window width
  /// \param height  OpenGL window height
  static bool out(const std::string& path, int width, int height);
};

