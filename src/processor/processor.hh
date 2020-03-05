#pragma once

#include "../state/state.hh"

class Processor
{
 public:
  State* state_;

  Processor(State* state);
};
