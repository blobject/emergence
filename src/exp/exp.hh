//===-- exp/exp.hh - Exp class declaration ---------------------*- C++ -*-===//
///
/// \file
/// Definitions of the Coloring enum and declaration of the Exp class, which
/// implements utilities for experimenting with the particle system, including,
/// for example, methods for counting and injecting particle clusters.
/// Exp directly accesses and modifies State.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../proc/proc.hh"
#include "../state/state.hh"
#include <set>
#include <unordered_map>
#include <unordered_set>


// Color: Type of particle coloring.

enum class Coloring
{
  Original = 0,
  Dynamic,
  Cluster,
  Inspect,
  Density10,
  Density15,
  Density20,
  Density25,
  Density30,
  Density35,
  Density40
};


typedef std::tuple<float,float,float,float,float> SpritePt;
typedef std::vector<SpritePt>                     SpritePts;


enum class Type;
class Proc;
class State;

class Exp
{
 public:
  /// constructor: Initialise experiment module.
  /// \param log  Log object
  /// \param state  State object
  /// \param proc  Proc object
  /// \param no_cl  whether user has specified disabling of OpenCL
  Exp(Log& log, State& state, Proc& proc, bool no_cl);

  /// init_experiment(): Modify State according to experiment being performed.
  /// \param experiment_group  experiment being performed
  /// \param experiment  specific experiment being performed
  void init_experiment(int experiment_group, int experiment);

  /// type(): Assign type to particle.
  void type();

  /// reset_exp(): Clear out all experimentation data structures.
  void reset_exp();

  /// reset_color(): Clear out all coloring-related data structures.
  void reset_color();

  /// reset_cluster(): Clear out clustering-related data structures.
  void reset_cluster();

  /// reset_inject(): Clear out injection-related data structures.
  void reset_inject();

  /// color(): Compute coloring of particles.
  /// \param scheme  particle coloring scheme
  void color(Coloring scheme);

  /// highlight(): Color specified particles brightly.
  /// \param particles  list of particle indices to highlight.
  void highlight(std::vector<unsigned int>& particles);

  /// cluster(): Detect particle clusters.
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  void cluster(float radius, unsigned int minpts);

  /// districts(): Detect greater (expanded) neighborhoods of clusters.
  void districts();

  /// inject(): Inject particle clusters.
  /// \param type  particle cluster type to be injected
  /// \param greater  whether the greater scope is to be injected
  void inject(Type type, bool greater);

  /// do_meta_exp(): Perform and report on meta-experiments.
  ///                Used for counting color classes and finding averages.
  /// \param tick  current time step
  void do_meta_exp(unsigned int tick);

  /// do_exp_*(): Perform and report on a experiment variation.
  /// \param tick  current time step
  /// \returns  (some) true if system should halt, change, etc.
  void do_exp_1a(unsigned int tick); // occupancy, t in {0,150}
  void do_exp_1b(unsigned int tick); // occupancy, t in {60,90,180,400,700}
  void do_exp_2(unsigned int tick);  // population
  void do_exp_3(unsigned int tick);  // heat map
  bool do_exp_4a(unsigned int tick); // survival, mature spore
  bool do_exp_4b(unsigned int tick); // survival, triangle cell
  bool do_exp_4c(unsigned int tick); // survival by reproduction
  bool do_exp_5a(unsigned int tick); // size, dpe in {0.03,0.035,0.04}
  bool do_exp_5b(unsigned int tick); // noise, dpe in {0.03,0.035,0.04}
  bool do_exp_6(unsigned int tick);  // param sweep, alpha & beta

  // experiment recurrence
  unsigned int exp_4_count_;
  unsigned int exp_5_count_;
  int exp_4_est_done_;
  int exp_4_dbscan_done_;
  std::string exp_4_est_how_;
  std::string exp_4_dbscan_how_;
  int exp_5_est_done_;
  int exp_5_dbscan_done_;
  std::string exp_5_est_how_;
  std::string exp_5_dbscan_how_;
  std::unordered_map<int,int> exp_5_est_size_counts_;
  std::unordered_map<int,int> exp_5_dbscan_size_counts_;
  // meta, nearest neighbors, color change, etc.
  unsigned int magentas_; // number of mature spore particles
  unsigned int blues_;    // number of cell hull particles
  unsigned int yellows_;  // number of cell core particles
  unsigned int browns_;   // number of premature spore particles
  unsigned int greens_;   // number of nutrient particles
  std::vector<float>             nearest_neighbor_dists_; // nn distances
  std::vector<std::vector<Type>> type_history_;           // type changes
  // clustering
  std::vector<int>           cores_;          // "core" particles
  std::vector<int>           vague_;          // "border" or "noise" pts
  std::vector<std::set<int>> clusters_;       // set of clusters
  std::unordered_set<int>    cell_clusters_;  // set of cell cluster indices
  std::unordered_set<int>    spore_clusters_; // set of spore cluster indices
  std::vector<std::set<int>> districts_;      // set of greater clusters
  // injection
  std::unordered_map<Type,SpritePts> sprites_;         // sprites definition
  std::unordered_map<Type,SpritePts> greater_sprites_; // greater sprites def
  float                              sprite_x_;        // sprite x placement
  float                              sprite_y_;        // sprite y placement
  std::vector<unsigned int>          injected_;        // injected particles

 private:
  /// plain_alt_neighborhood(): Non-OpenCL version of alternative neighborhood
  ///                           gathering.
  ///                           Count neighbors in an alternative radius.
  ///                           Used for coloring spores.
  /// \param pld  left neighbor distances
  /// \param prd  right neighbor distances
  /// \param p  particle index
  /// \param n_stride  neighbor list stride
  /// \param alt_scope  alternative radius squared
  /// \returns  number of neighbors within alternative radius
  unsigned int plain_alt_neighborhood(std::vector<float>& pld,
                                      std::vector<float>& prd,
                                      unsigned int p, unsigned int n_stride,
                                      float alt_scope);

  /// palette_sample(): Generate stack (cache) of random colors for clusters.
  /// \returns  set of random colors
  std::vector<float> palette_sample();

  /// nearest_neighbor_dists(): Compute nearest neighbor distances.
  void nearest_neighbor_dists();

  /// record_types(): Record type changes for every particle.
  void record_types();

  /// dbscan_categorise(): Compute neighborhoods of each particle and
  ///                      categorise them as either "core", "noise", or
  ///                      "vague", depending on their neighborhood size.
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  void dbscan_categorise(float radius, unsigned int minpts);

  /// dbscan_collect(): Use computed particle categories to accumulate the
  ///                   clusters.
  void dbscan_collect();

  /// type_clusters(): Assign type to particle cluster.
  void type_clusters();

  /// type_of_cluster(): Determine type of specified particle cluster.
  /// \param cluster  particle cluster
  /// \returns  -1 if mature spore, 1 if cell, 0 otherwise
  int type_of_cluster(std::set<int>& cluster);

  /// gen_sprite(): Generate an absolutely-positioned typical sprite that was
  ///               "captured" from prior runs.
  /// \param xyf  flat list of x,y positions and f heading
  /// \param num  number of particles in the cluster
  /// \returns  sprite definition that is a list of 5-float tuples
  SpritePts gen_sprite(std::vector<float> xyf, unsigned int num);

  /// gen_greater_sprite(): Generate an absolutely-positioned typical sprite
  ///                       that was "captured" from prior runs, along with
  ///                       all other particles within the sprite's scope.
  /// \param type  sprite (cluster) type
  /// \param xyf  flat list of x,y positions and f heading
  /// \param num  number of particles in the cluster
  /// \returns  sprite definition that is a list of 5-float tuples
  SpritePts gen_greater_sprite(Type type, std::vector<float> xyf,
                               unsigned int num);

  /// dhi(): Compute the density-homogeneity index.
  /// \returns  dhi
  float dhi();

  Log&   log_;
  State& state_;
  Proc&  proc_;
  bool                            no_cl_;
  std::vector<std::vector<float>> palette_;  // cluster color cache
  unsigned int                    palette_index_;
  std::vector<unsigned int>       inspect_;  // particles under inspection
};

