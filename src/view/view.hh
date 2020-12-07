#pragma once

#include <memory>

#include "../proc/proc.hh"
#include "../util/log.hh"


// View: Abstract class over the Canvas and Headless views.

class View
{
 public:
  virtual ~View() {}

  // factory method
  static std::unique_ptr<View> Init(Log &log, State &state, Proc &proc,
                                    bool visual, bool hide_ctrl);

  virtual void Exec() = 0;
};

