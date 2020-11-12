#pragma once

#include <memory>

#include "../sys/sys.hh"


// View: Abstract class over the Canvas and Headless views.

class View
{
 public:
  virtual ~View() {}

  // factory method
  static std::unique_ptr<View> Init(Sys* sys, bool visual, bool hidectrl);

  virtual void Exec() = 0;
};

