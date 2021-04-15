#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


State::State(Log& log)
  : log_(log)
{
  // transportable
  this->num_ = 5000;
  this->width_ = 250;
  this->height_ = 250;
  this->alpha_ = PI; // 180 degrees
  this->beta_ = 0.296705972839036L; // 17 degrees
  this->scope_ = 5.0f;
  this->ascope_ = 1.3f;
  this->speed_ = 0.67f;
  this->prad_ = 1.0f;
  this->coloring_ = 0;
  // derived
  this->scope_squared_ = this->scope_ * this->scope_;
  this->ascope_squared_ = this->ascope_ * this->ascope_;
  // fixed
  this->n_stride_ = 100;

  this->spawn();
}


void
State::spawn()
{
  float w = static_cast<float>(this->width_);
  float h = static_cast<float>(this->height_);
  unsigned int n_stride = this->n_stride_;
  for (int i = 0; i < this->num_; ++i) {
    this->px_.push_back(Util::dist(0.0f, w));
    this->py_.push_back(Util::dist(0.0f, h));
    this->pf_.push_back(Util::dist(0.0f, TAU));
    this->pc_.push_back(cosf(this->pf_[i]));
    this->ps_.push_back(sinf(this->pf_[i]));
    this->pn_.push_back(0);
    this->pl_.push_back(0);
    this->pr_.push_back(0);
    this->pan_.push_back(0);
    for (int j = 0; j < n_stride; ++j) { this->pls_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { this->prs_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { this->pld_.push_back(-1.0f); }
    for (int j = 0; j < n_stride; ++j) { this->prd_.push_back(-1.0f); }
    this->pt_.push_back(Type::None);
    this->gcol_.push_back(0);
    this->grow_.push_back(0);
    this->xr_.push_back(1.0f);
    this->xg_.push_back(1.0f);
    this->xb_.push_back(1.0f);
    this->xa_.push_back(0.5f);
  }
}


void
State::clear()
{
  this->px_.clear();
  this->py_.clear();
  this->pf_.clear();
  this->pc_.clear();
  this->ps_.clear();
  this->pn_.clear();
  this->pl_.clear();
  this->pr_.clear();
  this->pan_.clear();
  this->pls_.clear();
  this->prs_.clear();
  this->pld_.clear();
  this->prd_.clear();
  this->pt_.clear();
  this->gcol_.clear();
  this->grow_.clear();
  this->xr_.clear();
  this->xg_.clear();
  this->xb_.clear();
  this->xa_.clear();
}


inline void
State::respawn()
{
  this->clear();
  this->spawn();
}


void
State::change(Stative& input, bool respawn)
{
  this->num_      = input.num;
  this->width_    = input.width;
  this->height_   = input.height;
  this->alpha_    = input.alpha;
  this->beta_     = input.beta;
  this->scope_    = input.scope;
  this->ascope_   = input.ascope;
  this->speed_    = input.speed;
  this->prad_     = input.prad;
  this->coloring_ = input.coloring;
  this->scope_squared_ = input.scope * input.scope;
  this->ascope_squared_ = input.ascope * input.ascope;

  std::string message = "Changed state";
  if (respawn) {
    message += " with respawn.";
    this->respawn();
  } else {
    message += " without respawn.";
  }
  this->log_.add(Attn::O, message);
  this->notify(Issue::StateChanged); // Canvas reacts
}


std::string
State::type_name(Type type)
{
  return TypeNames[static_cast<int>(type)];
}

