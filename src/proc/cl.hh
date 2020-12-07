#pragma once

#include <CL/cl2.hpp>

#include "../state/state.hh"
#include "../util/log.hh"


// Cl: OpenCL.

class Cl
{
 private:
  Log             &log_;
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

  Cl(Log &log);
  //~Cl();

  bool Good();
  void PrepSeek();
  void Seek(std::vector<int> &grid, unsigned int gstride, unsigned int n,
            std::vector<float> &px, std::vector<float> &py,
            std::vector<float> &pc, std::vector<float> &ps,
            std::vector<unsigned int> &pn,
            std::vector<unsigned int> &pl, std::vector<unsigned int> &pr,
            std::vector<int> &pgcol, std::vector<int> &pgrow,
            int cols, int rows, unsigned int w, unsigned int h, float scope2);
  void PrepMove();
  void Move(unsigned int n, std::vector<float> &px, std::vector<float> &py,
            std::vector<float> &pf,
            std::vector<float> &pc, std::vector<float> &ps,
            std::vector<unsigned int> &pn,
            std::vector<unsigned int> &pl, std::vector<unsigned int> &pr,
            unsigned int w, unsigned int h, float a, float b, float s);
};

