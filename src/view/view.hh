#pragma once

#include "../processor/processor.hh"


// View: Abstract class over the Visualiser and Headless views.

class View
{
 public:
  // factory method
  static View* Init(Processor* processor, bool visual, bool hidectrl);

  virtual void Exec() = 0;
};

