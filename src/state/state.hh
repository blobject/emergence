#pragma once

#include <algorithm>
#include <string>
#include <vector>


// Distribution: For initial particle distribution

enum class Distribution { UNIFORM = 0, GAUSSIAN = 1 };


// History: Record of particle system instances.

class History
{
 public:
  std::vector<int> snapshots_; // cumulative

  History();
};


// StateTransport: Struct for transporting data from GuiState to State

struct StateTransport
{
  // stable
  Distribution distribution;
  unsigned int stop;
  // adjustable
  unsigned int num;
  unsigned int width;
  unsigned int height;
  float        alpha;
  float        beta;
  float        scope;
  float        speed;
  int          colorscheme;
};


// State: Main data source of the primordial particle system.

class State //: public Subject
{
 public:
  std::vector<float> px_;        // volatile
  std::vector<float> py_;        // volatile
  std::vector<float> pf_;        // volatile
  std::vector<float> ps_;        // volatile
  std::vector<float> pc_;        // volatile
  std::vector<unsigned int> pn_; // volatile
  std::vector<unsigned int> pl_; // volatile
  std::vector<unsigned int> pr_; // volatile
  std::vector<float> prad_;      // adjustable

  // sedentary data
  History history_;     // cumulative
  int     colorscheme_; // adjustable

  // transportable data
  Distribution distribution_; // (stable) initial position distribution
  unsigned int stop_;         // (stable) # iterations until process stop
  unsigned int num_;          // (adjustable) # particles
  unsigned int width_;        // (adjustable) processable space width (pixels)
  unsigned int height_;       // (adjustable) processable space height (pixels)
  float        scope_;        // (adjustable) neighborhood radius
  float        speed_;        // (adjustable) movement multiplier
  float        alpha_;        // (adjustable) alpha in main formula (degrees)
  float        beta_;         // (adjustable) beta in main formula (degrees)

  // derived data
  unsigned int          half_width_;
  unsigned int          half_height_;
  float                 scope_squared_;

  State(const std::string &path);

  void Spawn();
  bool Change(StateTransport &next);
  void Respawn();
};

