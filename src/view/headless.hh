#pragma once

#include "view.hh"
#include "../proc/proc.hh"
#include "../util/log.hh"
#include "../util/observation.hh"


// Headless: The non-graphical (commandline-only) view.

class Headless : public View, Observer
{
 private:
  Log  &log_;  // also the subject being observed
  Proc &proc_;

 public:
  Headless(Log &log, Proc &proc);
  ~Headless() override;

  void Exec() override;
  void React(Subject&) override;
};

