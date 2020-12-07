#pragma once

#include "cl.hh"
#include "../state/state.hh"
#include "../util/log.hh"
//#include "../util/observation.hh"


// Proc: Primordial particle system algorithm implementation.

class Proc
{
 private:
  Log             &log_;
  Cl*              cl_;
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
  bool   cl_good_;

  Proc(Log &log, State &state);
  //~Proc() override;

  //void React(Subject&) override;

  //inline State &get_state() const { return this->state_; }
  void Next();
};

