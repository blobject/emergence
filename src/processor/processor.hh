#pragma once

#include "../state/state.hh"


// Processor: Primordial particle system algorithm implementation.

class Processor
{
 public:
  State* state_;

  Processor(State* state);

  static void InitCl();
  void        Process();
};

