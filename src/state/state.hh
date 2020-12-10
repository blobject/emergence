#pragma once

#include <string>
#include <vector>

#include "../proc/control.hh"
#include "../util/log.hh"


// Mutually include-dependent classes.

class Stative;


// State: Main data source of the primordial particle system.

class State : public Subject
{
 private:
  Log &log_;

 public:
  /// particle data
  // location & direction
  std::vector<float> px_;        // volatile
  std::vector<float> py_;        // volatile
  std::vector<float> pf_;        // volatile
  std::vector<float> pc_;        // volatile
  std::vector<float> ps_;        // volatile
  // neighborhood
  std::vector<unsigned int> pn_; // volatile
  std::vector<unsigned int> pl_; // volatile
  std::vector<unsigned int> pr_; // volatile
  // visualisation
  std::vector<float> prad_;      // adjustable
  // grid
  std::vector<int> pgcol_;       // volatile
  std::vector<int> pgrow_;       // volatile

  // sedentary data
  int colors_;                   // adjustable

  // transportable data
  int          num_;    // # particles (negative for error)
  unsigned int width_;  // processable space width (pixels)
  unsigned int height_; // processable space height (pixels)
  float        alpha_;  // alpha in main formula (degrees)
  float        beta_;   // beta in main formula (degrees)
  float        scope_;  // neighborhood radius
  float        speed_;  // movement multiplier

  // derived data
  float scope_squared_;

  State(Log &log);

  void Spawn();
  void Respawn();
  void Clear();
  bool Change(Stative &gui);
};

