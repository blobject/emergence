#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


State::State(Log& log, int e)
  : log_(log), experiment_(e)
{
  // transportable
  this->num_      = 5000; // 0.08 dpe
  this->width_    = 250;
  this->height_   = 250;
  this->alpha_    = Util::deg_to_rad(180.0f);
  this->beta_     = Util::deg_to_rad(17.0f);
  this->scope_    = 5.0f;
  this->ascope_   = 1.3f;
  this->speed_    = 0.67f;
  this->noise_    = 0.0f;
  this->prad_     = 1.0f;
  this->coloring_ = 0;
  // derived
  this->scope_squared_ = this->scope_ * this->scope_;
  this->ascope_squared_ = this->ascope_ * this->ascope_;
  // fixed
  this->n_stride_ = 100;

  int eg = 10 <= e ? e / 10 : e;
  this->experiment_group_ = eg;
  if (1 == eg) {
    this->width_  = 50;
    this->height_ = 50;
    this->prad_   = 0.5f;
    if      (11 == e) { this->num_ = 12; }   // custom placement
    else if (12 == e) { this->num_ = 14; }   // custom placement
    else if (13 == e) { this->num_ = 100; } // 0.04 dpe
    else if (14 == e) { this->num_ = 175; } // 0.07 dpe
    else if (15 == e) { this->num_ = 225; } // 0.09 dpe
  } else if (2 == eg) {
    this->num_    = 5000; // 0.08 dpe
    this->width_  = 250;
    this->height_ = 250;
    this->prad_   = 1.0f;
  } else if (3 == eg) {
    this->num_    = 0; // variable dpe, inj
    this->width_  = 250;
    this->height_ = 250;
    this->prad_   = 0.5f;
  } else if (4 == eg) {
    this->width_  = 50;
    this->height_ = 50;
    this->num_    = static_cast<unsigned int>(this->width_ * this->height_
                                              * 0.0f); // variable dpe, inj
    this->prad_   = 0.25f;
  } else if (5 == eg) {
    this->width_  = 50;
    this->height_ = 50;
    this->prad_   = 0.25f;
    if (51 == e || 54 == e) {
      this->num_ = static_cast<unsigned int>(this->width_ * this->height_
                                             * 0.03f); // 0.03 dpe, inj
    } else if (52 == e || 55 == e) {
      this->num_ = static_cast<unsigned int>(this->width_ * this->height_
                                             * 0.035f); // ~0.035 dpe, inj
    } else if (53 == e || 56 == e) {
      this->num_ = static_cast<unsigned int>(this->width_ * this->height_
                                             * 0.04f); // 0.04 dpe, inj
    }
  } else if (6 == eg) {
    this->num_    = 1200; // 0.12 dpe, param sweep
    this->width_  = 100;
    this->height_ = 100;
    this->alpha_  = Util::deg_to_rad(-180.0f);
    this->beta_   = Util::deg_to_rad(-60.0f);
    this->prad_   = 0.5f;
  } else if (e) {
    log.add(Attn::E, "state will ignore unknown experiment " +
            std::to_string(e), true);
  }

  this->spawn();

  log.add(Attn::O, "Started state module.", true);
}


void
State::spawn()
{
  float w = static_cast<float>(this->width_);
  float h = static_cast<float>(this->height_);
  unsigned int num = this->num_;
  unsigned int n_stride = this->n_stride_;
  int e = this->experiment_;

  if (11 == e || 12 == e) {
    float center = w / 2.0f;
    float spread = 2.5f;
    float min = center - spread;
    float max = center + spread;
    for (int i = 0; i < num; ++i) {
      this->px_.push_back(Util::distr(min, max));
      this->py_.push_back(Util::distr(min, max));
    }
  } else {
    for (int i = 0; i < num; ++i) {
      this->px_.push_back(Util::distr(0.0f, w));
      this->py_.push_back(Util::distr(0.0f, h));
    }
  }
  for (int i = 0; i < num; ++i) {
    this->pf_.push_back(Util::distr(0.0f, TAU));
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
  this->noise_    = input.noise;
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


float
State::dpe()
{
  return static_cast<float>(this->num_) / this->width_ / this->height_;
}

