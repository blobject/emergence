#pragma once

#include "view.hh"
#include "../processor/processor.hh"


// Headless: The non-graphical (commandline-only) view.

class Headless : public View
{
 public:
  Processor* processor_;

  Headless(Processor* processor);

  void Exec() override;
};

