#pragma once

#include "view.hh"
#include "../proc/proc.hh"


// Headless: The non-graphical (commandline-only) view.

class Headless : public View
{
 public:
  Proc* proc_;

  Headless(Proc* proc);

  void Exec() override;
};

