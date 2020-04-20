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
  float radius;   // adjustable

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

/**
// Observer pattern for State

class Subject;

class Observer
{
 public:
  virtual ~Observer() = default;
  virtual void Update(Subject& s) = 0;
};

class Subject
{
 private:
  std::vector<Observer*> observers;

 public:
  virtual ~Subject() = default;

  void Register(Observer& o);
  void Deregister(Observer& o);
  void Notify();
};
//*/


// State: Main data source of the primordial particle system.

class State //: public Subject
{
 public:

  // sedentary data
  History history_;     // cumulative
  int     colorscheme_; // adjustable

  // transportable data
  unsigned int          num_;          // adjustable
  unsigned int          width_;        // adjustable
  unsigned int          half_width_;
  unsigned int          height_;       // adjustable
  unsigned int          half_height_;
  float                 nradius_;      // adjustable
  float                 nradius_squared_;
  float                 speed_;        // adjustable
  float                 alpha_;        // adjustable
  float                 beta_;         // adjustable
  float                 gamma_;        // adjustable
  Distribution          distribution_; // stable
  unsigned int          stop_;         // stable
  std::vector<Particle> particles_;    // volatile/adjustable

  State(const std::string &path);

  void set_num(unsigned int num);
  void set_dim(unsigned int width, unsigned int height);
  void set_nradius(float nradius);
  void set_speed(float speed);
  void set_alpha(float alpha);
  void set_beta(float beta);
  void set_gamma(float gamma);
  //void set_distribution(Distribution distribution);
  //void set_stop(unsigned int stop);
};

