#include <chrono>
#include <iostream>

#include "state.hh"
#include "../util/util.hh"


Particle::Particle(std::default_random_engine rng,
                   Distribution distribution,
                   unsigned int width,
                   unsigned int height)
{
  this->x_ = 0;
  this->y_ = 0;
  this->phi_ = 0.0;
  this->size_ = 0;
  this->speed_ = 0.0;
  this->neighsize_ = 0.0;
  if (distribution == Distribution::UNIFORM)
  {
    this->x_ = std::uniform_int_distribution<int>(0, width - 1)(rng);
    this->y_ = std::uniform_int_distribution<int>(0, height - 1)(rng);
    this->phi_ = std::uniform_real_distribution<float>(0, 1)(rng);
  }
}


History::History()
{
  this->snapshots_ = std::vector<int>(100, 0);
}


State::State(const std::string &load)
{
  // sedentary data
  this->rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
  this->history_ = new History();
  this->colorscheme_ = 0;

  // transportable data
  this->width_ = 800;
  this->height_ = 800;
  this->alpha_ = 0.0;
  this->beta_ = 0.0;
  this->gamma_ = 0.0;
  this->distribution_ = Distribution::UNIFORM;
  this->stop_ = 0;
  this->particles_ = std::vector<Particle>
    (1000, Particle(this->rng_, this->distribution_,
                    this->width_, this->height_));

  if (! load.empty())
  {
    if (! Util::LoadState(this, load))
    {
      std::cerr << "Warning: could not load file '" << load << "'" << std::endl;
    }
  }
}

State::~State()
{
  delete this->history_;
}

