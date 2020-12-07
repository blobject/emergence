#pragma once

#include <string>
#include <vector>

#include "../util/log.hh"
#include "../util/observation.hh"


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
  History history_;     // cumulative
  int     colorscheme_; // adjustable

  // transportable data
  unsigned int stop_;         // (stable) # iterations until process stop
  unsigned int num_;          // (adjustable) # particles
  unsigned int width_;        // (adjustable) processable space width (pixels)
  unsigned int height_;       // (adjustable) processable space height (pixels)
  float        alpha_;        // (adjustable) alpha in main formula (degrees)
  float        beta_;         // (adjustable) beta in main formula (degrees)
  float        scope_;        // (adjustable) neighborhood radius
  float        speed_;        // (adjustable) movement multiplier

  // derived data
  float scope_squared_;

  State(Log &log, const std::string &path);

  void Spawn();
  void Respawn();
  void Clear();
  bool Change(StateTransport &next);
  bool Save(const std::string &path);
  bool Load(const std::string &path);
};

