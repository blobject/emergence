#pragma once

#include "view.hh"
#include "../util/log.hh"


// When: Type of report

enum class When { Done = 0, Paused };


// Headless: The non-graphical (commandline) View.

class Headless : public View, Observer
{
  public:
    Headless(Log& log, Control& ctrl);
    ~Headless() override;

    void exec() override;
    void react(Issue issue) override;
    void tell_pause() const;
    void report(When when) const;
    static void sigint_callback(int signal);

  private:
    Log&     log_;
    Control& ctrl_;
    bool     paused_;
};

