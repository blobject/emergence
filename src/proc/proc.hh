//===-- proc.hh - Proc class declaration ------------------------*- C++ -*-===//
///
/// \file
/// Declaration of the Proc class, which implements the core particle system
/// processing algorithms, including particle vicinity seeking and particle
/// movement behavior. For OpenCL variants of the algorithms, the Cl class is
/// invoked.
/// Proc directly accesses State and is controlled by Control.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "cl.hh"
#include "../state/state.hh"
#include "../util/log.hh"


class Cl;
class State;

class Proc : public Subject
{
 public:
  /// constructor: Initialise system processing.
  /// \param log  Log object
  /// \param state  State object
  /// \param cl  Cl object
  /// \param no_cl  whether user has specified disabling of OpenCL
  Proc(Log& log, State& state, Cl& cl, bool no_cl);

  /// next(): Let the system perform one action step.
  void next();

  /// done(): Pause the system and notify Views.
  inline void
  done()
  {
    this->paused_ = true;
    this->notify(Issue::ProcDone); // Views react
  }

  State& state_;
  bool   paused_;  // whether processing is paused
  bool   cl_good_; // retain value of Cl::good()

 private:
  /// plot(): Prepare seek() and move() (for either OpenCL or plain versions).
  ///         Namely, clear out seek data and (re)generate the grid.
  void plot();

#if 1 == CL_ENABLED

  /// seek(): Entry point for OpenCL version of seek.
  ///         Calculate new N, L, R (seek data) for each particle.
  void seek();

  /// move(): Entry point for OpenCL version of move.
  ///         Update to new X, Y, PHI (move data) for each particle.
  void move();

#endif /* CL_ENABLED */

  /// plain_seek(): Non-OpenCL version of seek.
  ///               Iterate through each particle.
  void plain_seek();

  /// plain_seek_vicinity(): For the non-OpenCL version of seek.
  ///                        Iterate through every other particle in the
  ///                        vicinity, ie. the 3x3 neighboring subset of the
  ///                        grid centered around src.
  /// \param grid  flat vector representing the grid
  /// \param stride  stride between each grid unit
  /// \param col  grid columns vector
  /// \param row  grid rows vector
  /// \param cols  number of grid columns
  /// \param rows  number of grid rows
  /// \param srci  index of the source particle
  void plain_seek_vicinity(std::vector<int>& grid, unsigned int stride,
                           int col, int row, int cols, int rows, int srci);

  /// plain_seek_tally(): For the non-OpenCL version of seek.
  ///                     Update N, L, R of the two particles provided
  ///                     respectively by the two foregoing callers.
  /// \param srci  index of the source particle
  /// \param dsti  index of the destination particle
  /// \param cunder  whether the column is underflowing
  /// \param cover  whether the column is overflowing
  /// \param runder  whether the row is underflowing
  /// \param rover  whether the row is overflowing
  void plain_seek_tally(unsigned int srci, unsigned int dsti,
                        bool cunder, bool cover, bool runder, bool rover);

  /// plain_move(): Non-OpenCL version of move.
  ///               Update X, Y, PHI of every particle.
  void plain_move();

  Cl&              cl_; // NOTE: if a pointer instead, clCreateBuffer fails
  std::vector<int> grid_;        // flat vector of the vicinity overlay grid
  int              grid_cols_;   // number of grid columns
  int              grid_rows_;   // number of grid rows
  unsigned int     grid_stride_; // size of a flattened grid unit
};

