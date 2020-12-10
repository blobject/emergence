#pragma once

#include "view.hh"
#include "../util/log.hh"


// Headless: The non-graphical (commandline) View.

class Headless : public View, Observer
{
 private:
  Log     &log_;
  Control &ctrl_;

 public:
  Headless(Log &log, Control &ctrl);
  ~Headless() override;

  void Exec() override;
  void React(Topic topic) override;
};

