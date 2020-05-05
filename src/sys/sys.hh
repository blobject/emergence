#pragma once

#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

class Sys
{
 private:
  std::vector<std::vector<std::vector<unsigned int> > > grid_;
  std::vector<Particle> old_particles_;

  void InitGrid();
  void Reset();
  void Regrid();
  void Mirror();
  void Seek();
  void Move();

 public:
  State &state_;

  Sys(State &state);
  //~Sys();

  //inline State &get_state() const { return this->state_; }
  void Next();
};

