#pragma once

#include <memory>

#include "../proc/control.hh"
#include "../util/log.hh"


// View: Abstract class over the Canvas and Headless views.

class View
{
 public:
  virtual ~View() {}

  // factory method
  static std::unique_ptr<View> Init(Log &log, Control &ctrl,
                                    bool headless, bool hide_side);

  virtual void Exec() = 0;
};

