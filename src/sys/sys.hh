#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>

#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

using Grid = std::vector<std::vector<std::vector<unsigned int> > >;

class Sys
{
 private:
  Grid grid_;
  cl::Platform cl_platform_;
  cl::Device cl_device_;

  void InitCl();
  void InitGrid();
  void Reset();
  void Regrid();
  void Seek();
  void SeekFrom(std::vector<Particle> &ps, Grid &grid,
                unsigned int cols, unsigned int rows);
  void SeekTo(std::vector<Particle> &ps, Grid &grid,
              unsigned int col,  unsigned int row,
              unsigned int cols, unsigned int rows,
              unsigned int srci);
  void SeekTally(std::vector<Particle> &ps,
                 unsigned int srci, unsigned int dsti);
  void Move();

 public:
  State &state_;

  Sys(State &state);
  //~Sys();

  //inline State &get_state() const { return this->state_; }
  void Next();
};

