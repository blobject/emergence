#pragma once

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

  Visualiser(Processor* processor, bool hide_ctrl);

  void Exec() override;
};

