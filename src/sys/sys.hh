#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>

#include "../state/state.hh"


// Sys: Primordial particle system algorithm implementation.

class Sys
{
 private:
  std::vector<std::vector<std::vector<unsigned int> > > grid_;
  std::vector<Particle> old_particles_;
  cl::Platform cl_platform_;
  cl::Device cl_device_;

  void InitCl();
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

