#pragma once

#include "../processor/processor.hh"

class Headless
{
 public:
  Processor* proc_;

  Headless(Processor* proc);

  void Process();
};
