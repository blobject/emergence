#pragma once

#include <algorithm>
#include <string>
#include <vector>


// Distribution: For initial particle distribution

enum class Distribution { UNIFORM = 0, GAUSSIAN = 1 };


// Particle: A particle in the primordial particle system.

struct Particle
{
  // transportable data
  float x;        // volatile
  float y;        // volatile
  float phi;      // volatile
  unsigned int n; // volatile
  unsigned int l; // volatile
  unsigned int r; // volatile
  float s;        // volatile
  float c;        // volatile
  float size;     // adjustable

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


// StateTransport: Struct for transporting data from GuiState to State

struct StateTransport
{
  // stable
  Distribution distribution;
  unsigned int stop;
  // adjustable
  int          colorscheme;
  unsigned int num;
  unsigned int width;
  unsigned int height;
  float        scope;
  float        speed;
  float        alpha;
  float        beta;
};


// State: Main data source of the primordial particle system.

class State //: public Subject
{
 public:

  // sedentary data
  History history_;     // cumulative
  int     colorscheme_; // adjustable

  // transportable data
  Distribution          distribution_; // stable
  unsigned int          stop_;         // stable
  unsigned int          num_;          // adjustable
  unsigned int          width_;        // adjustable
  unsigned int          height_;       // adjustable
  float                 scope_;        // adjustable
  float                 speed_;        // adjustable
  float                 alpha_;        // adjustable
  float                 beta_;         // adjustable
  std::vector<Particle> particles_;    // volatile/adjustable

  // derived data
  unsigned int          half_width_;
  unsigned int          half_height_;
  float                 scope_squared_;

  State(const std::string &path);

  void Change(StateTransport &next);
  void Respawn();
};

