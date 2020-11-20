#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Particle::Particle(Distribution distribution,
                   unsigned int width,
                   unsigned int height)
{
  this->rad = 2.0f;
  if (distribution == Distribution::UNIFORM)
  {
    this->x = Util::Distribute<float>(0.0f, static_cast<float>(width));
    this->y = Util::Distribute<float>(0.0f, static_cast<float>(height));
    this->phi = Util::Distribute<float>(0.0f, TAU);
    this->n = 0;
    this->l = 0;
    this->r = 0;
    this->s = sinf(this->phi);
    this->c = cosf(this->phi);
  }
}


History::History()
{
  this->snapshots_ = std::vector<int>(100, 0);
}


State::State(const std::string &load)
{
  // sedentary
  this->history_ = History();
  this->colorscheme_ = 0;

  // transportable
  this->distribution_ = Distribution::UNIFORM;
  this->stop_ = 0;
  this->num_ = 4000;
  this->width_ = 1000;
  this->height_ = 1000;
  this->alpha_ = PI; // 180 degrees
  this->beta_ =  0.296705972839036L; // 17 degrees
  this->scope_ = 24.0f;
  this->speed_ = 4.0f;
  for (int i = 0; i < this->num_; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }

  // derived
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
bool
State::Change(StateTransport &next)
{
  bool respawn = false;

  if (next.distribution != this->distribution_ ||
      next.width        != this->width_        ||
      next.height       != this->height_)
  {
    respawn = true;
  }

  if (! respawn &&
      next.num         == this->num_         &&
      next.alpha       == this->alpha_       &&
      next.beta        == this->beta_        &&
      next.scope       == this->scope_       &&
      next.speed       == this->speed_       &&
      next.colorscheme == this->colorscheme_)
  {
    return false;
  }

  this->distribution_ = next.distribution;
  this->stop_ = next.stop;
  this->num_ = next.num;
  this->width_ = next.width;
  this->height_ = next.height;
  this->alpha_ = next.alpha;
  this->beta_ = next.beta;
  this->scope_ = next.scope;
  this->speed_ = next.speed;
  this->colorscheme_ = next.colorscheme;

  // derived
  this->half_width_ = next.width / 2;
  this->half_height_ = next.height / 2;
  this->scope_squared_ = next.scope * next.scope;

  if (respawn)
  {
    this->Respawn();
  }

  return true;
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

