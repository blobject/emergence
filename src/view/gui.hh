#pragma once

#include <GLFW/glfw3.h>

#include "../state/state.hh"


// Gui: The part of the visualisation dealing with the user interface.

class Gui
{
 private:
  double ago_;
  unsigned int frames_;
  unsigned int fps_;

 public:
  GLFWwindow* window_;

  Gui(const std::string &version, unsigned int width, unsigned int height);
  ~Gui();

  void Close();
  bool Closing() const;
  void Draw(const State &state);
  void HandleInput();
  void Next() const;
};

