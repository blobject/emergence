#pragma once

#include <GLFW/glfw3.h>

#include "gui.hh"
#include "view.hh"
#include "../processor/processor.hh"


// Visualiser: The graphical view.

class Visualiser : public View
{
 public:
  Processor*   processor_;
  Gui*         gui_;
  unsigned int width_;
  unsigned int height_;

  Visualiser(Processor* processor_, bool hide_ctrl);
  ~Visualiser();

  void Exec();
};

