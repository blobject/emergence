//===-- view.hh - View class declaration ------------------------*- C++ -*-===//
///
/// \file
/// Declaration of the View class, which provides a view over the particle
/// system.
/// It is an abstract class over the Canvas and Headless classes.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../proc/control.hh"
#include "../util/log.hh"
#include <memory>


class View
{
  public:
    virtual ~View() {}

    /// init(): Factory of views.
    /// \param log  Log object
    /// \param ctrl Control object
    /// \param headless  whether view is non-graphical
    /// \param hide_side  whether to hide the control side bar
    /// \returns a View, either Canvas or Headless
    static std::unique_ptr<View> init(Log& log, Control& ctrl,
                                      bool headless, bool hide_side);

    /// exec(): Reaction to one iteration of particle processing.
    virtual void exec() = 0;
};

