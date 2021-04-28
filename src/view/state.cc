#include "state.hh"


UiState::UiState(Control& ctrl)
  : ctrl_(ctrl)
{
  State& truth    = ctrl.get_state();
  this->duration_ = ctrl.duration_;
  this->num_      = truth.num_;
  this->width_    = truth.width_;
  this->height_   = truth.height_;
  this->alpha_    = Util::rad_to_deg(truth.alpha_);
  this->beta_     = Util::rad_to_deg(truth.beta_);
  this->scope_    = truth.scope_;
  this->ascope_   = truth.ascope_;
  this->speed_    = truth.speed_;
  this->noise_    = Util::rad_to_deg(truth.noise_);
  this->prad_     = truth.prad_;
  this->coloring_ = static_cast<Coloring>(truth.coloring_);
  this->pattern_  = 0;
}


Stative
UiState::current() const
{
  return {this->duration_,
          this->num_,
          this->width_,
          this->height_,
          Util::deg_to_rad(this->alpha_),
          Util::deg_to_rad(this->beta_),
          this->scope_,
          this->ascope_,
          this->speed_,
          Util::deg_to_rad(this->noise_),
          this->prad_,
          static_cast<int>(this->coloring_)};
}


int
UiState::untrue() const
{
  Control& ctrl = this->ctrl_;
  State& truth = ctrl.get_state();
  Stative current = this->current();

  if (current.num    != truth.num_   ||
      current.width  != truth.width_ ||
      current.height != truth.height_)
  {
    return -1;
  }

  if (current.duration != ctrl.duration_                ||
      !Util::floats_same(current.alpha,  truth.alpha_)  ||
      !Util::floats_same(current.beta,   truth.beta_)   ||
      !Util::floats_same(current.scope,  truth.scope_)  ||
      !Util::floats_same(current.ascope, truth.ascope_) ||
      !Util::floats_same(current.speed,  truth.speed_)  ||
      !Util::floats_same(current.noise,  truth.noise_)  ||
      !Util::floats_same(current.prad,   truth.prad_))
  {
    return 1;
  }

  return 0;
}


void
UiState::deceive(bool respawn /* = false */) const
{
  Stative current = this->current();
  if (!respawn && 0 > this->untrue()) {
    respawn = true;
  }
  // truth change provokes Canvas (observer) reaction
  this->ctrl_.change(current, respawn);
}


void
UiState::receive()
{
  Control& ctrl = this->ctrl_;
  State& truth = ctrl.get_state();
  this->duration_ = ctrl.duration_;
  this->num_      = truth.num_;
  this->width_    = truth.width_;
  this->height_   = truth.height_;
  this->alpha_    = Util::rad_to_deg(truth.alpha_);
  this->beta_     = Util::rad_to_deg(truth.beta_);
  this->scope_    = truth.scope_;
  this->ascope_   = truth.ascope_;
  this->speed_    = truth.speed_;
  this->noise_    = Util::rad_to_deg(truth.noise_);
  this->prad_     = truth.prad_;
  this->coloring_ = static_cast<Coloring>(truth.coloring_);
}


std::string
UiState::random()
{
  float scale = std::min(this->width_, this->height_) / 100.0f;
  this->alpha_ = Util::distr(-180.0f, 180.0f);
  this->beta_ = Util::distr(-180.0f, 180.0f);
  this->scope_ = Util::distr(1.0f, 10.0f * scale);
  //this->ascope_ = Util::distr(1.0f, 5.0f * scale);
  //this->noise_ = Util::distr(-90.0f, 90.0f);
  this->speed_ = Util::distr(1.0f, scale);

  std::ostringstream message;
  message << "Preset: random (a=" << std::to_string(this->alpha_)
          << ", b=" + std::to_string(this->beta_)
          << ", v=" + std::to_string(this->scope_)
          << ", s=" + std::to_string(this->speed_) << ").";

  return message.str();
}


std::string
UiState::pattern()
{
  std::string which;
  int pattern = this->pattern_;
  if (0 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = 17.0f;
    which = "Lifelike structures 1";
  } else if (1 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = -7.0f;
    which = "Moving structures";
  } else if (2 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = -15.0f;
    which = "Clean cow pattern";
  } else if (3 == pattern) {
    this->alpha_ = 90.0f; this->beta_ = -21.0f;
    which = "Chaos w/ random aggr. 1";
  } else if (4 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -10.0f;
    which = "Fingerprint pattern";
  } else if (5 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -41.0f;
    which = "Chaos w/ random aggr. 2";
  } else if (6 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -25.0f;
    which = "Untidy cow pattern";
  } else if (7 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = -48.0f;
    which = "Chaos w/ random aggr. 3";
  } else if (8 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = 5.0f;
    which = "Regular pattern";
  } else if (9 == pattern) {
    this->alpha_ = -159.0f; this->beta_ = 15.0f;
    which = "Lifelike structures 2";
  } else if (10 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 1.0f;
    which = "Stable cluster pattern";
  } else if (11 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = 58.0f;
    which = "Chaotic pattern 1";
  } else if (12 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 40.0f;
    which = "Chaotic pattern 2";
  } else if (13 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 8.0f;
    which = "Cells & moving cluster";
  } else if (14 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 0.0f;
    which = "Chaotic pattern 3";
  } else if (15 == pattern) {
    this->alpha_ = 45.0f; this->beta_ = 4.0f;
    which = "Stable rings";
  }

  std::ostringstream message;
  message << "Preset: " << which << " (a="
          << std::to_string(this->alpha_)
          << ", b=" << std::to_string(this->beta_) << ").";

  return message.str();
}


bool
UiState::save(const std::string& path)
{
  return this->ctrl_.save(path);
}


bool
UiState::load(const std::string& path)
{
  Control& ctrl = this->ctrl_;
  Stative loaded = ctrl.load(path); // truth has changed

  if (-1 == loaded.num) {
    return false;
  }

  this->duration_ = loaded.duration;
  this->num_      = loaded.num;
  this->width_    = loaded.width;
  this->height_   = loaded.height;
  this->alpha_    = Util::rad_to_deg(loaded.alpha);
  this->beta_     = Util::rad_to_deg(loaded.beta);
  this->scope_    = loaded.scope;
  this->ascope_   = loaded.ascope;
  this->speed_    = loaded.speed;
  this->noise_    = loaded.noise;
  this->prad_     = loaded.prad;
  this->coloring_ = static_cast<Coloring>(loaded.coloring);

  return true;
}

