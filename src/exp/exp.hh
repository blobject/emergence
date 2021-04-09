//===-- exp/exp.hh - Exp class declaration ---------------------*- C++ -*-===//
///
/// \file
/// Definitions of the Coloring and TypeBit enums and declaration of the Exp
/// class, which implements utilities for experimenting with the particle
/// system, including, for example, methods for counting and injecting particle
/// clusters.
/// Exp directly accesses and modifies State.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../state/state.hh"
#include <unordered_map>
#include <unordered_set>


// TypeBit: Type of particle, as a bitfield.

enum class TypeBit
{
  PrematureSpore = 1<<0,
  MatureSpore    = 1<<1,
  Nutrient       = 1<<2,
  CellHull       = 1<<3,
  CellCore       = 1<<4,
};


// Color: Type of particle coloring.

enum class Coloring
{
  Original = 0,
  Dynamic,
  Cluster,
  Density10,
  Density15,
  Density20,
  Density25,
  Density30
};


typedef std::tuple<float,float,float,float,float> SpritePt;
typedef std::vector<SpritePt>                     SpritePts;


enum class Type;
class State;

class Exp
{
 public:
  /// constructor: Initialise experiment module.
  /// \param log  Log object
  /// \param state  State object
  Exp(Log& log, State& state);

  /// type(): Assign type to particle.
  void type();

  /// type_name(): Get name of a cluster type.
  ///              Assumes that the Type enum is continuous.
  /// \param type  particle cluster type
  /// \returns  name of cluster type
  std::string type_name(Type type);

  /// coloring(): Compute particle coloring.
  /// \param scheme  particle coloring scheme
  void coloring(Coloring scheme);

  /// reset_exp(): Clear out all experimentation data structures.
  void reset_exp();

  /// reset_cluster(): Clear out clustering-related data structures.
  void reset_cluster();

  /// reset_inject(): Clear out injection-related data structures.
  void reset_inject();

  /// cluster(): Detect particle clusters.
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  void cluster(float radius, unsigned int minpts);

  /// inject(): Inject particle clusters.
  /// \param type  particle cluster type to be injected
  /// \param dpe  DPE after injection
  void inject(Type type, float dpe);

  std::unordered_map<int,std::vector<int>> neighbor_sets_; // set of nbhds
  std::vector<int>                     cores_;    // "core" particles
  std::vector<int>                     vague_;    // "border" or "noise" pts
  std::vector<std::unordered_set<int>> clusters_; // set of clusters
  std::unordered_map<Type,SpritePts>   sprites_;  // sprites definition
  unsigned int cell_clusters_;  // number of clusters that are cells
  unsigned int spore_clusters_; // number of clusters that are spores
  unsigned int sprite_index_;   // cursor to state.p*_
  float        sprite_scale_;   // sprite scaling
  float        sprite_x_;       // sprite x placement
  float        sprite_y_;       // sprite y placement

 private:
  /// alt_neighborhood(): Count neighbors in an alternative radius.
  ///                     Used for coloring spores.
  /// \param pnd  neighbors distance list
  /// \param p  particle index
  /// \param stride  neighbors list stride
  /// \param alt_radius_squared  alternative radius squared
  /// \returns  number of neighbors in alternative radius
  unsigned int alt_neighborhood(std::vector<float>& pnd, unsigned int p,
                                unsigned int stride, float alt_radius_squared);

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

  /// is_cluster_type(): Whether a cluster is of a specific type.
  /// \param target  particle type(s) in bits
  /// \param cluster  particle cluster
  /// \returns  true if cluster is of specified type
  bool is_cluster_type(TypeBit target, std::unordered_set<int>& cluster);

  /// cluster_type_count(): Count clusters by type.
  void cluster_type_count();

  /// gen_sprite(): Generate an absolutely-positioned sprite with initially
  ///               randomly headed constituent particles.
  /// \param xy  flat list of x,y positions
  /// \param num  number of particles in the cluster
  /// \returns  sprite definition that is a list of 5-float tuples
  SpritePts gen_sprite(std::vector<float> xy, unsigned int num);

  Log&   log_;
  State& state_;
  std::vector<std::vector<float>>    palette_; // cluster color cache
  unsigned int                       palette_index_;
};

