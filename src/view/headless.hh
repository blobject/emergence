#pragma once

#include "view.hh"
#include "../sys/sys.hh"


// Headless: The non-graphical (commandline-only) view.

class Headless : public View
{
 public:
  Sys* sys_;

  Headless(Sys* sys);

  void Exec() override;
};

