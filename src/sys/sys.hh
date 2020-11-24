#pragma once

#include "cl.hh"
#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

using Grid = std::vector<std::vector<std::vector<unsigned int> > >;

class Sys
{
 private:
  Cl*  cl_;
  Grid grid_;
  bool cl_good_;

  void GenGrid();
  void Reset();
  void Seek();
  void Move();
  void PlainSeek();
  void PlainMove();
  void SeekFrom(Grid &grid, unsigned int cols, unsigned int rows);
  void SeekTo(Grid &grid,
              unsigned int col,  unsigned int row,
              unsigned int cols, unsigned int rows,
              unsigned int srci);
  void SeekTally(unsigned int srci, unsigned int dsti,
                 bool c_under, bool c_over, bool r_under, bool r_over);

 public:
  State &state_;

  Sys(State &state);
  //~Sys();

  //inline State &get_state() const { return this->state_; }
  void Next();
};

