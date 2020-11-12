#pragma once

#include <CL/cl2.hpp>

#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

using Grid = std::vector<std::vector<std::vector<unsigned int> > >;

class Sys
{
 private:
  Grid grid_;
  cl::Platform cl_platform_;

  void InitCl();
  void InitGrid();
  void ExecCl();
  void Reset();
  void Regrid();
  void Seek();
  void ClSeek();
  void SeekFrom(std::vector<Particle> &ps, Grid &grid,
                unsigned int cols, unsigned int rows);
  void SeekTo(std::vector<Particle> &ps, Grid &grid,
              unsigned int col,  unsigned int row,
              unsigned int cols, unsigned int rows,
              unsigned int srci);
  void SeekTally(std::vector<Particle> &ps,
                 unsigned int srci, unsigned int dsti,
                 bool c_under, bool c_over, bool r_under, bool r_over);
  void Move();

 public:
  State &state_;
  cl::Device cl_device_;

  Sys(State &state);
  //~Sys();

  //inline State &get_state() const { return this->state_; }
  void Next();
};

