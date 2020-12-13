#pragma once

#include "../proc/control.hh"
#include "../util/log.hh"
#include <memory>


// View: Abstract class over the Canvas and Headless views.

class View
{
  public:
    virtual ~View() {}

    // factory method
    static std::unique_ptr<View> init(Log& log, Control& ctrl,
                                      bool headless, bool hide_side);

    virtual void exec() = 0;
};

