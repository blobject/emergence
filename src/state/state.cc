#include <cmath>

#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Particle::Particle(Distribution distribution,
                   unsigned int width,
                   unsigned int height)
{
  this->size = 5.0f;
  if (distribution == Distribution::UNIFORM)
  {
    this->x = Util::Distribute<float>(0.0f, static_cast<float>(width));
    this->y = Util::Distribute<float>(0.0f, static_cast<float>(height));
    this->phi = Util::Distribute<float>(0.0f, TAU);
    this->n = 0;
    this->l = 0;
    this->r = 0;
    this->s = sin(this->phi);
    this->c = cos(this->phi);
  }
}


History::History()
{
  this->snapshots_ = std::vector<int>(100, 0);
}


State::State(const std::string &load)
{
  // sedentary data
  this->history_ = History();
  this->colorscheme_ = 0;

  // transportable data
  this->distribution_ = Distribution::UNIFORM;
  this->stop_ = 0;
  this->num_ = 4000;
  this->width_ = 1000;
  this->height_ = 1000;
  this->scope_ = 100.0f;
  this->alpha_ = 3.141593f;
  this->beta_ = 0.296706f;
  this->speed_ = 2.65f;
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }

  // derived data
  this->half_width_ = this->width_ / 2;
  this->half_height_ = this->height_ / 2;
  this->scope_squared_ = this->scope_ * this->scope_;

  if (! load.empty())
  {
    if (! Util::LoadState(*this, load))
    {
      Util::Err("could not load file '" + load + "'");
    }
  }
}


// Change: Update State data.
void
State::Change(StateTransport &next)
{
  bool respawn = false;
  if (next.distribution != this->distribution_ ||
      next.num          != this->num_          ||
      next.width        != this->width_        ||
      next.height       != this->height_)
  {
    respawn = true;
  }
  this->distribution_ = next.distribution;
  this->stop_ = next.stop;
  this->colorscheme_ = next.colorscheme;
  this->num_ = next.num;
  this->width_ = next.width;
  this->height_ = next.height;
  this->scope_ = next.scope;
  this->speed_ = next.speed;
  this->alpha_ = next.alpha;
  this->beta_ = next.beta;
  if (respawn)
  {
    this->Respawn();
  }
}


// Respawn: Reinitialise particles.
void
State::Respawn()
{
  this->particles_.clear();
  //std::vector<Particle>().swap(this->particles_); // resize to fit
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }
}

