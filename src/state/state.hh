#pragma once

#include <random>

#include "../util/common.hh"


// Particle: A particle in the primordial particle system.

class Particle
{
 public:
  // transportable data
  unsigned int x_;         // volatile
  unsigned int y_;         // volatile
  float        phi_;       // volatile
  float        speed_;     // adjustable
  unsigned int size_;      // adjustable
  float        neighsize_; // adjustable

  Particle(std::default_random_engine rng,
           Distribution distribution,
           unsigned int width,
           unsigned int height);
};


// History: Record of (almost) all particle instances.

class History
{
 public:
  std::vector<int> snapshots_; // cumulative

  History();
};


// State: Main data source of the primordial particle system.

class State
{
 public:
  // sedentary data
  std::default_random_engine rng_;         // stable
  History*                   history_;     // cumulative
  int                        colorscheme_; // adjustable

  // transportable data
  unsigned int          width_;        // adjustable
  unsigned int          height_;       // adjustable
  float                 alpha_;        // adjustable
  float                 beta_;         // adjustable
  float                 gamma_;        // adjustable
  Distribution          distribution_; // stable
  unsigned int          stop_;         // stable
  std::vector<Particle> particles_;    // volatile/adjustable

  State(const std::string &path);
  ~State();
};

