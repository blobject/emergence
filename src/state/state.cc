#include <cmath>

#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Particle::Particle(Distribution distribution,
                   unsigned int width,
                   unsigned int height)
{
  this->radius = 5.0f;
  if (distribution == Distribution::UNIFORM)
  {
    this->x = Util::Distribute<float>(0.0f, (float) width);
    this->y = Util::Distribute<float>(0.0f, (float) height);
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


/**
void
Subject::Register(Observer& o)
{
  this->observers.push_back(&o);
}


void
Subject::Deregister(Observer& o)
{
  this->observers.erase(std::remove(this->observers.begin(),
                                    this->observers.end(),
                                    &o));
}


void
Subject::Notify()
{
  for (auto* o : this->observers)
  {
    o->Update(*this);
  }
}
//*/


State::State(const std::string &load)
{
  // sedentary data
  this->history_ = History();
  this->colorscheme_ = 0;

  // transportable data
  this->num_ = 3000;
  this->width_ = 1200;
  this->half_width_ = this->width_ / 2;
  this->height_ = 1200;
  this->half_height_ = this->height_ / 2;
  this->nradius_ = 100.0f;
  this->nradius_squared_ = this->nradius_ * this->nradius_;
  this->alpha_ = 3.141593f;
  this->beta_ = 0.296706f;
  this->gamma_ = 0.05f;
  this->speed_ = this->nradius_ * this->gamma_;
  this->distribution_ = Distribution::UNIFORM;
  this->stop_ = 0;
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }

  if (! load.empty())
  {
    if (! Util::LoadState(*this, load))
    {
      Util::Err("could not load file '" + load + "'");
    }
  }
}


void
State::set_num(unsigned int num)
{
  this->num_ = num;
  this->particles_.clear();
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }
  //this->Notify();
}


void
State::set_dim(unsigned int width, unsigned int height)
{
  this->width_ = width;
  this->height_ = height;
  this->particles_.clear();
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }
  //this->Notify();
}


void
State::set_alpha(float alpha)
{
  this->alpha_ = alpha;
  //this->Notify();
}


void
State::set_beta(float beta)
{
  this->beta_ = beta;
  //this->Notify();
}


void
State::set_gamma(float gamma)
{
  this->gamma_ = gamma;
  //this->Notify();
}


void
State::set_nradius(float nradius)
{
  this->nradius_ = nradius;
  this->nradius_squared_ = nradius * nradius;
  //this->Notify();
}


void
State::set_speed(float speed)
{
  this->speed_ = speed;
  //this->Notify();
}

