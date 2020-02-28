#pragma once

#include <GLFW/glfw3.h>

class View
{
 public:
  GLFWwindow* window;
  unsigned int width;
  unsigned int height;

  View();
  GLFWwindow* Init(unsigned int width, unsigned int height);
  void        End();
  int         Draw(GLFWwindow* window);
};
