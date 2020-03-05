#pragma once

#include <random>
#include <string>
#include <vector>

#include "../util/common.hh"

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

class History
{
 public:
  std::vector<int> snapshots_; // cumulative

  History();
};

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

  State(std::string path);

  void Fin();
};

