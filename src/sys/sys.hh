#pragma once

#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

class Sys //: public Observer
{
 private:
  State &state_;
  std::vector<std::vector<std::vector<unsigned int> > > grid_;

  static void InitCl();
  void        InitGrid();
  void        Reset();
  void        Regrid();
  void        Seek();
  void        Move();

 public:
  Sys(State &state);
  //~Sys();

  inline State &get_state() const { return this->state_; }
  void Next();
};

