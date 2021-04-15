//===-- state/state.hh - State class declaration ---------------*- C++ -*-===//
///
/// \file
/// Definition of the Type enum and declaration of the State class, which acts
/// as the main data store for the particle system.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../proc/control.hh"
#include "../util/log.hh"
#include <string>
#include <vector>


// Type: Type of particle, by its vicinity/neighborhood/local density.
//       While a type gets assigned per particle, this enum can also be used to
//       refer to particle groups/structures/clusters.
//       Should be continuous for TypeNames[].

enum class Type
{
  None = 0,
  PrematureSpore, // coloring and injecting
  MatureSpore,    // coloring and injecting
  Ring,           // injecting
  PrematureCell,  // injecting
  TriangleCell,   // injecting
  SquareCell,     // injecting
  PentagonCell,   // injecting
  Nutrient,       // coloring
  CellHull,       // coloring
  CellCore        // coloring
};

static const std::string TypeNames[] = {
  "(none)",
  "premature spore",
  "mature spore",
  "ring",
  "premature cell",
  "triangle cell",
  "square cell",
  "pentagon cell",
  "nutrient",
  "cell hull",
  "cell core"
};


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

  /// clear(): Clear out the particle parameters.
  void clear();

  /// respawn(): Reinitialise the particle parameters.
  void respawn();

  /// change(): Mutate the system parameters.
  /// \param input  system parameters to change to
  /// \param respawn  whether system should respawn
  void change(Stative& input, bool respawn);

  /// type_name(): Get name of a particle type.
  ///              Assumes that the Type enum is continuous.
  /// \param type  particle type
  /// \returns  name of particle type
  std::string type_name(Type type);

  //// particle
  // (volatile) location & direction
  std::vector<float> px_;         // X parameter
  std::vector<float> py_;         // Y parameter
  std::vector<float> pf_;         // PHI parameter
  std::vector<float> pc_;         // cos(PHI) parameter
  std::vector<float> ps_;         // sin(PHI) parameter
  // (volatile) vicinity
  std::vector<unsigned int> pn_;  // N(=L+R) parameter
  std::vector<unsigned int> pl_;  // L parameter
  std::vector<unsigned int> pr_;  // R parameter
  std::vector<unsigned int> pan_; // alternative N parameter (for spores)
  std::vector<int>          pls_; // L neighbor indices (signed!)
  std::vector<int>          prs_; // R neighbor indices (signed!)
  std::vector<float>        pld_; // L neighbor distances
  std::vector<float>        prd_; // R neighbor distances
  std::vector<Type>         pt_;  // type (nutrient, mature spore, ring, etc.)
  // (volatile) grid
  std::vector<int> gcol_;         // grid column the particle is in
  std::vector<int> grow_;         // grid row the particle is in
  // (volatile) color
  std::vector<float> xr_;         // red
  std::vector<float> xg_;         // green
  std::vector<float> xb_;         // blue
  std::vector<float> xa_;         // opacity

  // transportable
  int          num_;      // # particles (negative for encoding input error)
  unsigned int width_;    // processable space width
  unsigned int height_;   // processable space height
  float        alpha_;    // alpha in main formula (radians)
  float        beta_;     // beta in main formula (radians)
  float        scope_;    // vicinity radius
  float        ascope_;   // alternative vicinity radius
  float        speed_;    // movement multiplier
  float        prad_;     // particle radius
  int          coloring_; // particle color scheme (int)

  // derived
  float scope_squared_;
  float ascope_squared_;

  // fixed
  unsigned int n_stride_; // neighbor list stride

 private:
  Log& log_;
};

