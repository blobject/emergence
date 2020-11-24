#pragma once

#include <memory>

#include "../proc/proc.hh"


// View: Abstract class over the Canvas and Headless views.

class View
{
 public:
  virtual ~View() {}

  // factory method
  static std::unique_ptr<View> Init(Proc* proc, bool visual, bool hidectrl);

  virtual void Exec() = 0;
};

