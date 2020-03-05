#pragma once

#include "headless.hh"
#include "visualiser.hh"
#include "../processor/processor.hh"

class View
{
 public:
  bool visual_;
  Visualiser* visualiser_;
  Headless* headless_;

  View(Processor* proc, bool visual, bool hidectrl);

  void Fin();
};
