//===-- exp.hh - Exp class declaration -------------------------*- C++ -*-===//
///
/// \file
/// Declaration of the Exp class, which implements utilities for experimenting
/// with the particle system, including for example methods for counting and
/// injecting particle clusters.
/// Exp directly accesses State in order to read world state.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../state/state.hh"
#include <map>
#include <unordered_map>
#include <unordered_set>


// Color: Type of particle coloring.

enum class Coloring { Normal = 0, Cluster,
                      Density10, Density15, Density20, Density25, Density30,
                      Half };


class State;

class Exp
{
 public:
  /// constructor: Initialise experiment module.
  /// \param log  Log object
  /// \param state  State object
  Exp(Log& log, State& state);

  void reset();
  void coloring(Coloring scheme);
  std::string cluster();
  std::string cluster2(float radius, unsigned int minpts);
  std::string inject();
  std::string densities();

 private:
  std::vector<float> random_color();
  void hdbscan();

  void dbscan_categorise(float radius, unsigned int minpts);
  void dbscan_neighborhood(float radius);
  void dbscan_neighborhood2(float radius);
  void dbscan_collect();

  Log&                            log_;
  State&                          state_;
  std::unordered_map<int,std::vector<int>>  neighbor_sets_;
  std::vector<int>                cores_;
  std::vector<int>                ambiguous_;
  std::vector<std::unordered_set<int>>   clusters_;
  std::unordered_map<int,std::vector<int>>  cluster_index_sets_;
  std::vector<std::vector<float>> random_colors_;
  unsigned int                    random_color_index_;
};

