//===-- proc/cl.hh - Cl class declaration ----------------------*- C++ -*-===//
///
/// \file
/// Declaration of the (optional) Cl class, which provides the Proc class with
/// processing variants that leverage OpenCL to perform parallel computation on
/// the GPU.
/// If OpenCL is not available, this becomes a stub class which Proc will
/// effectively ignore.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../util/log.hh"

#if 1 == CL_ENABLED

#include <CL/cl2.hpp>

#endif /* CL_ENABLED */


class Cl
{
 public:

#if 1 == CL_ENABLED

  /// constructor: Initialise the OpenCL device and build the computation
  ///              kernels.
  /// \param log  Log object
  Cl(Log& log);

  /// prep_seek(): Pre-build the kernel for performing particle seeking.
  ///              See Proc::plain_seek(), plain_seek_vicinity(), and
  ///              plain_seek_tally() for the non-OpenCL variants.
  void prep_seek();

  /// seek: Perform particle seeking.
  /// \param n  number of particles
  /// \param w  width of the particle system
  /// \param h  width of the particle system
  /// \param scope  vicinity radius squared
  /// \param ascope  alternative vicinity radius squared
  /// \param cols  number of grid columns
  /// \param rows  number of grid rows
  /// \param grid_stride  stride between each grid unit
  /// \param grid  flat vector representing the grid
  /// \param gcol  grid columns vector
  /// \param grow  grid rows vector
  /// \param px  X particle parameter vector
  /// \param py  Y particle parameter vector
  /// \param pc  cos(PHI) particle parameter vector
  /// \param ps  sin(PHI) particle parameter vector
  /// \param pn  N particle parameter vector
  /// \param pan  alternative N particle parameter vector
  /// \param pl  L particle parameter vector
  /// \param pr  R particle parameter vector
  void seek(unsigned int n, unsigned int w, unsigned int h, float scope,
            float ascope, int cols, int rows, unsigned int grid_stride,
            std::vector<int>& grid,
            std::vector<int>& gcol, std::vector<int>& grow,
            std::vector<float>& px, std::vector<float>& py,
            std::vector<float>& pc, std::vector<float>& ps,
            std::vector<unsigned int>& pn, std::vector<unsigned int>& pan,
            std::vector<unsigned int>& pl, std::vector<unsigned int>& pr);

  /// prep_move(): Pre-build the kernel for performing particle moving.
  ///              See Proc::plain_move() for the non-OpenCL variant.
  void prep_move();

  /// move: Perform particle moving.
  /// \param n  number of particles
  /// \param w  width of the particle system
  /// \param h  height of the particle system
  /// \param a  alpha parameter
  /// \param b  beta parameter
  /// \param s  speed parameter
  /// \param e  noise parameter
  /// \param pn  N particle parameter vector
  /// \param pl  L particle parameter vector
  /// \param pr  R particle parameter vector
  /// \param px  X particle parameter vector
  /// \param py  Y particle parameter vector
  /// \param pf  PHI particle parameter vector
  /// \param pc  cos(PHI) particle parameter vector
  /// \param ps  sin(PHI) particle parameter vector
  void move(unsigned int n, unsigned int w, unsigned int h,
            float a, float b, float s, float e,
            std::vector<unsigned int>& pn,
            std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
            std::vector<float>& px, std::vector<float>& py,
            std::vector<float>& pf,
            std::vector<float>& pc, std::vector<float>& ps);

  /// good(): Whether OpenCL is enabled.
  /// \returns  true if OpenCL is enabled
  bool
  good() const
  {
    return !this->device_.getInfo<CL_DEVICE_NAME>().empty();
  }

#else

  /// constructor: Stub (OpenCL unavailable).
  /// \param log  (unused) Log object
  inline Cl(Log& /* log */) {}

  /// good(): Stub (OpenCL unavailable).
  /// \returns  false
  inline bool
  good() const
  {
    return false;
  }

#endif /* CL_ENABLED else */

#if 1 == CL_ENABLED

 private:
  Log&             log_;
  cl::Platform     platform_;
  cl::Device       device_;
  cl::Context      context_;
  cl::CommandQueue queue_;
  cl::Kernel       kernel_seek_;
  cl::Kernel       kernel_move_;
  unsigned int     max_cu_;   // max GPU compute units
  unsigned int     max_freq_; // max GPU frequency
  unsigned int     max_gmem_; // max global memory

#endif /* CL_ENABLED */

};

