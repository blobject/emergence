#pragma once

#include <string>
#include <vector>

#include "../util/common.hh"


// Particle: A particle in the primordial particle system.

struct Particle
{
  // transportable data
  float x_;         // volatile
  float y_;         // volatile
  float phi_;       // volatile
  float speed_;     // adjustable
  float size_;      // adjustable
  float neighsize_; // adjustable

  Particle(Distribution distribution,
           unsigned int width,
           unsigned int height);
};


// History: Record of particle system instances.

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
  History history_;     // cumulative
  int     colorscheme_; // adjustable

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
  //~State();
};

