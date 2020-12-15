//===-- headless.hh - Headless class declaration ----------------*- C++ -*-===//
///
/// \file
/// Definition of the When enum and declaration of the Headless class, which is
/// responsible for non-graphical (commandline) access to the particle system.
/// It is a subclass of View and observes the Proc and Control classes.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "view.hh"
#include "../util/log.hh"


// When: Type of report.

enum class When { Done = 0, Paused };


class Headless : public View, Observer
{
  public:
    /// constructor: Start observing Proc and Control, prepare signal handling,
    ///              and notify user about pausing.
    /// \param log  Log object
    /// \param ctrl  Control object
    Headless(Log& log, Control& ctrl);

    /// destructor: Detach from observation.
    ~Headless() override;

    /// exec(): Take account of iterations by printing the tick count.
    ///         This is a virtual function of the View superclass.
    void exec() override;

    /// react(): React to Log::add(), Proc::next(), Proc::done().
    ///          This is a virtual function of the Observer class.
    /// \param issue  which observed Subject's function to react to.
    void react(Issue issue) override;

    /// tell_pause(): Print message about pausing.
    void tell_pause() const;

    /// report(): Print processed information.
    /// \param when  whether processing is paused or done.
    void report(When when) const;

    /// sigint_callback(): Handle Ctrl-C.
    /// \param signal  signal
    static void sigint_callback(int signal);

  private:
    Log&     log_;
    Control& ctrl_;
};

