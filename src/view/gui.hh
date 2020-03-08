#pragma once

#include <GLFW/glfw3.h>

#include "../state/state.hh"


// Gui: The part of the visualisation dealing with the user interface.

class Gui
{
 public:
  GLFWwindow* window_;

  Gui(const std::string &version, unsigned int width, unsigned int height);
  ~Gui();

  void Close();
  bool Closing() const;
  void Draw(const State &state) const;
  void HandleInput();
  void Next() const;
};

