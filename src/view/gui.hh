#pragma once

#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


// Gui: The part of the visualisation dealing with the user interface.

class Gui
{
 public:
  GLFWwindow* window_;

  Gui(const std::string &version, unsigned int width, unsigned int height);
  ~Gui();

  void Close();
  bool Closing();
  void Draw();
  void HandleInput();
};

