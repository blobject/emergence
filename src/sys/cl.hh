#pragma once

#include <CL/cl2.hpp>

#include "../state/state.hh"


// Cl: OpenCL.

class Cl
{
 private:
  cl::Platform     platform_;
  cl::Device       device_;
  cl::Context      context_;
  cl::CommandQueue queue_;
  unsigned int     max_cu_;
  unsigned int     max_freq_;
  unsigned int     max_gmem_;
  cl::Kernel       kernel_seek_;
  cl::Kernel       kernel_move_;

 public:

  Cl();
  //~Cl();

  bool Good();
  void PrepSeek();
  void PrepMove();
  void Seek();
  void Move(unsigned int num, std::vector<float> &px, std::vector<float> &py,
            std::vector<float> &pf,
            std::vector<float> &ps, std::vector<float> &pc,
            std::vector<unsigned int> &pn,
            std::vector<unsigned int> &pl, std::vector<unsigned int> &pr,
            unsigned int w, unsigned int h, float a, float b, float s);
  void Foo();
};

