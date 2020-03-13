#pragma once

#include <memory>

#include "../processor/processor.hh"


// View: Abstract class over the Visualiser and Headless views.

class View
{
 public:
  virtual ~View() {}

  // factory method
  static std::unique_ptr<View> Init(Processor* processor,
                                    bool visual, bool hidectrl);

  virtual void Exec() = 0;
};

