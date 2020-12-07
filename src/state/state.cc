#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


History::History()
{
  this->snapshots_ = std::vector<int>(100, 0);
}


State::State(Log &log, const std::string &load)
  : log_(log)
{
  // sedentary
  this->history_ = History();
  this->colorscheme_ = 0;

  // transportable
  this->stop_ = 0;
  this->num_ = 4000;
  this->width_ = 1000;
  this->height_ = 1000;
  this->alpha_ = PI; // 180 degrees
  this->beta_ = 0.296705972839036L; // 17 degrees
  this->scope_ = 24.0f;
  this->speed_ = 4.0f;

  // derived
  this->scope_squared_ = this->scope_ * this->scope_;

  this->Spawn(); // particles

  if (! load.empty())
  {
    if (! Util::LoadState(*this, load))
    {
      log.Add(Attn::E, "could not load file '" + load + "'");
    }
  }
}


// Spawn: Initialise particles.
void
State::Spawn()
{
  unsigned int w = this->width_;
  unsigned int h = this->height_;
  float rad = 2.0f;
  for (int i = 0; i < this->num_; ++i)
  {
    this->px_.push_back(Util::Distribute<float>(0.0f, static_cast<float>(w)));
    this->py_.push_back(Util::Distribute<float>(0.0f, static_cast<float>(h)));
    this->pf_.push_back(Util::Distribute<float>(0.0f, TAU));
    this->pc_.push_back(cosf(this->pf_[i]));
    this->ps_.push_back(sinf(this->pf_[i]));
    this->pn_.push_back(0);
    this->pl_.push_back(0);
    this->pr_.push_back(0);
    this->prad_.push_back(rad);
    this->pgcol_.push_back(0);
    this->pgrow_.push_back(0);
  }
}


// Respawn: Reinitialise particles.
void
State::Respawn()
{
  this->px_.clear();
  this->py_.clear();
  this->pf_.clear();
  this->pc_.clear();
  this->ps_.clear();
  this->pn_.clear();
  this->pl_.clear();
  this->pr_.clear();
  this->prad_.clear();
  this->pgcol_.clear();
  this->pgrow_.clear();
  this->Spawn();
}


// Change: Mutate State data.
bool
State::Change(StateTransport &next)
{
  bool respawn = false;

  if (next.width        != this->width_        ||
      next.height       != this->height_       ||
      next.num          != this->num_)
  {
    respawn = true;
  }

  if (! respawn &&
      next.alpha       == this->alpha_       &&
      next.beta        == this->beta_        &&
      next.scope       == this->scope_       &&
      next.speed       == this->speed_       &&
      next.colorscheme == this->colorscheme_)
  {
    return false;
  }

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
  this->scope_squared_ = next.scope * next.scope;

  if (respawn)
  {
    this->log_.Add(Attn::O, "Changing state and respawning.");
    this->Respawn();
  }
  else
  {
    this->log_.Add(Attn::O, "Changing state without respawn.");
  }

  // provoke reaction in Proc and View
  this->Notify();
  return true;
}

