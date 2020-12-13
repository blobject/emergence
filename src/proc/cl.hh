#pragma once

#include "../state/state.hh"
#include "../util/log.hh"
#include <CL/cl2.hpp>


// Cl: For computing on the GPU using OpenCL.

class Cl
{
  public:
    Cl(Log& log);
    void prep_seek();
    void seek(std::vector<int>& grid, unsigned int gstride, unsigned int n,
              std::vector<float>& px, std::vector<float>& py,
              std::vector<float>& pc, std::vector<float>& ps,
              std::vector<unsigned int>& pn,
              std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
              std::vector<int>& pgcol, std::vector<int>& pgrow,
              int cols, int rows, unsigned int w, unsigned int h, float scope2);
    void prep_move();
    void move(unsigned int n, std::vector<float>& px, std::vector<float>& py,
              std::vector<float>& pf,
              std::vector<float>& pc, std::vector<float>& ps,
              std::vector<unsigned int>& pn,
              std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
              unsigned int w, unsigned int h, float a, float b, float s);

    // good: OpenCL enabled?
    inline bool
    good() const
    {
        return !this->device_.getInfo<CL_DEVICE_NAME>().empty();
    }

  private:
    Log&             log_;
    cl::Platform     platform_;
    cl::Device       device_;
    cl::Context      context_;
    cl::CommandQueue queue_;
    unsigned int     max_cu_;
    unsigned int     max_freq_;
    unsigned int     max_gmem_;
    cl::Kernel       kernel_seek_;
    cl::Kernel       kernel_move_;
};

