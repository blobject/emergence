#pragma once

#include <GLFW/glfw3.h>

#include "../processor/processor.hh"

class Visualiser
{
 public:
  GLFWwindow* window_;
  unsigned int width_;
  unsigned int height_;

  Visualiser(Processor* proc, bool hide_ctrl);

  GLFWwindow* Init(unsigned int width, unsigned int height);
  void        Fin();
  int         Draw(GLFWwindow* window);
};
