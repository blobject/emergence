//===-- exp.hh - Exp class declaration -------------------------*- C++ -*-===//
///
/// \file
/// Declaration of the Exp class, which implements utilities for experimenting
/// with the particle system, including, for example, methods for counting and
/// injecting particle clusters.
/// Exp directly accesses State in order to read world state.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../state/state.hh"
#include <unordered_map>
#include <unordered_set>


// Color: Type of particle coloring.

enum class Coloring { Normal = 0, Cluster,
                      Density10, Density15, Density20, Density25, Density30 };


class State;

class Exp
{
 public:
  /// constructor: Initialise experiment module.
  /// \param log  Log object
  /// \param state  State object
  Exp(Log& log, State& state);

  /// reset(): Clear out analytics data structures.
  void reset();

  /// coloring(): Compute particle coloring.
  /// \param scheme  particle coloring scheme
  void coloring(Coloring scheme);

  /// cluster(): Detect particle clusters.
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  /// \returns  analysis result message
  std::string cluster(float radius, unsigned int minpts);

  /// inject(): Inject particle clusters.
  std::string inject();

  /// TODO
  std::string densities();

 private:
  /// palette_sample(): Generate stack (cache) of random colors for clusters.
  /// \returns  set of random colors
  std::vector<float> palette_sample();

  /// dbscan_categorise(): Compute neighborhoods of each particle and
  ///                      categorise them as either "core", "noise", or
  ///                      "vague", depending on their neighborhood size.
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  void dbscan_categorise(float radius, unsigned int minpts);

  /// dbscan_collect(): Use computed particle categories to accumulate the
  ///                   clusters.
  void dbscan_collect();

  /// dbscan_neighborhood(): TODO: merge with PPS plot & seek
  void dbscan_neighborhood(float radius);

  Log&   log_;
  State& state_;
  std::unordered_map<int,std::vector<int>> neighbor_sets_; // set of nbhds
  std::vector<int>                     cores_;    // "core" particles
  std::vector<int>                     vague_;    // "border" or "noise" pts
  std::vector<std::unordered_set<int>> clusters_; // set of clusters
  std::vector<std::vector<float>>      palette_;  // cluster color cache
  unsigned int                         palette_index_;
};

