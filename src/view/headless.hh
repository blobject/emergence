//===-- view/headless.hh - Headless class declaration ----------*- C++ -*-===//
///
/// \file
/// Definition of the When enum and declaration of the Headless class, which is
/// responsible for non-graphical (commandline) access to the particle system.
/// Headless is a subclass of the View class and observes the Proc and Control
/// classes.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "view.hh"
#include "../util/log.hh"


// When: Type of report.

enum class When
{
  Done = 0,
  Paused
};


class Headless : public View, Observer
{
 public:
  /// constructor: Start observing Proc and Control, prepare signal handling,
  ///              and notify user about how to interact.
  /// \param log  Log object
  /// \param ctrl  Control object
  /// \param uistate  UiState object
  Headless(Log& log, Control& ctrl, UiState& uistate);

  /// destructor: Detach from observation.
  ~Headless() override;

  /// intro(): Print message about pausing before starting process loop.
  void intro() override;

  /// exec(): Take account of iterations by printing the tick count.
  void exec() override;

  /// react(): React to Log::add(), Proc::next(), Proc::done().
  /// \param issue  which observed Subject's function to react to
  void react(Issue issue) override;

  /// tell_usage(): Print message about pausing.
  void tell_usage() const;

  /// report(): Print processed information.
  /// \param when  whether processing is paused or done
  void report(When when) const;

  /// prompt_*(): Produce menu-based prompts and handle input.
  void prompt_base();
  void prompt_config();
  void prompt_analyse();
  void prompt_inject() const;
  void prompt_inspect();

  /// tell_config(): Print current config (state).
  /// \returns  current state
  std::string tell_config() const;

  /// prompt_file(): Produce a filename prompt.
  /// \param ask  prompt message
  /// \returns  user input value
  std::string prompt_file(const std::string& ask) const;

  /// prompt_int(): Produce an int param config prompt.
  /// \param ask  prompt message
  /// \param low  lower limit
  /// \param high  higher limit
  /// \param low_inc  whether lower limit inclusive
  /// \param high_inc  whether higher limit inclusive
  /// \returns  user input value
  long prompt_int(const std::string& ask, unsigned int low, unsigned int high,
                  bool low_inc, bool high_inc) const;

  /// prompt_float(): Produce a float param config prompt.
  /// \param ask  prompt message
  /// \param low  lower limit
  /// \param high  higher limit
  /// \param low_inc  whether lower limit inclusive
  /// \param high_inc  whether higher limit inclusive
  /// \returns  user input value
  float prompt_float(const std::string& ask, float low, float high,
                     bool low_inc, bool high_inc) const;

  /// sigint_callback(): Handle Ctrl-C.
  /// \param signal  signal
  static void sigint_callback(int signal);

 private:
  Log&     log_;
  Control& ctrl_;
  UiState& uistate_;

  std::string current_;
};

