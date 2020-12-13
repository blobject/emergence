#pragma once

#include "view.hh"
#include "../util/log.hh"


// Headless: The non-graphical (commandline) View.

class Headless : public View, Observer
{
  public:
    Headless(Log& log, Control& ctrl);
    ~Headless() override;

    void exec() override;
    void react(Topic topic) override;

  private:
    Log&     log_;
    Control& ctrl_;
};

