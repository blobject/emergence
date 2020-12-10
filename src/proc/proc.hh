#pragma once

#include "cl.hh"
#include "../util/log.hh"


class Cl;
class State;


// Proc: Implementation of the particle system's core algorithm.

class Proc : public Subject
{
 private:
  Log             &log_;
  Cl              &cl_;
  std::vector<int> grid_;
  int              grid_cols_;
  int              grid_rows_;
  unsigned int     grid_stride_; // (max) size of a flattened grid unit

  void Plot();
  void Seek();
  void Move();
  void PlainSeek();
  void PlainSeekVicinity(std::vector<int> &grid, unsigned int stride,
                         int col, int row, int cols, int rows,
                         unsigned int srci);
  void PlainSeekTally(unsigned int srci, unsigned int dsti,
                      bool cunder, bool cover, bool runder, bool rover);
  void PlainMove();

 public:
  State &state_;
  bool   paused_;
  bool   cl_good_;

  Proc(Log &log, State &state, Cl &cl);

  void Next();
  void Done();
};

