//===-- state/state.hh - State class declaration ---------------*- C++ -*-===//
///
/// \file
/// Declaration of the State class, which acts as the main data store for the
/// particle system.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../proc/control.hh"
#include "../util/log.hh"
#include <string>
#include <vector>


struct Stative; // from control.hh

class State : public Subject
{
 public:
  /// constructor: Initialise the system and particles with default
  ///              parameters.
  /// \param log  Log object
  State(Log& log);

  /// spawn(): Initialise the particle parameters.
  void spawn();

  /// respawn(): Reinitialise the particle parameters.
  void respawn();

  /// clear(): Clear out the particle parameters.
  void clear();

  /// change(): Mutate the system parameters.
  /// \param input  system parameters to change to
  /// \param respawn  whether system should respawn
  void change(Stative& input, bool respawn);

  //// particle
  // (volatile) location & direction
  std::vector<float> px_;        // X parameter
  std::vector<float> py_;        // Y parameter
  std::vector<float> pf_;        // PHI parameter
  std::vector<float> pc_;        // cos(PHI) parameter
  std::vector<float> ps_;        // sin(PHI) parameter
  // (volatile) vicinity
  std::vector<unsigned int> pn_; // N(=L+R) parameter
  std::vector<unsigned int> pl_; // L parameter
  std::vector<unsigned int> pr_; // R parameter
  // (volatile) grid
  std::vector<int> gcol_;        // grid column the particle is in
  std::vector<int> grow_;        // grid row the particle is in
  // (volatile) color
  std::vector<float> xr_;        // red
  std::vector<float> xg_;        // green
  std::vector<float> xb_;        // blue

  // transportable
  int          num_;      // # particles (negative for encoding input error)
  unsigned int width_;    // processable space width
  unsigned int height_;   // processable space height
  float        alpha_;    // alpha in main formula (radians)
  float        beta_;     // beta in main formula (radians)
  float        scope_;    // vicinity radius
  float        speed_;    // movement multiplier
  float        prad_;     // particle radius
  int          coloring_; // particle color scheme (int)

  // derived
  float scope_squared_;

 private:
  Log& log_;
};

