#include "state.hh"


UiState::UiState(Control& ctrl)
  : ctrl_(ctrl)
{
  State& truth    = ctrl.get_state();
  this->stop_     = ctrl.stop_;
  this->num_      = truth.num_;
  this->width_    = truth.width_;
  this->height_   = truth.height_;
  this->alpha_    = Util::rad_to_deg(truth.alpha_);
  this->beta_     = Util::rad_to_deg(truth.beta_);
  this->scope_    = truth.scope_;
  this->speed_    = truth.speed_;
  this->prad_     = truth.prad_;
  this->coloring_ = (Coloring)truth.coloring_;
  this->pattern_   = 0;
}


Stative
UiState::current() const
{
  // TODO: stop
  return {this->stop_,
          this->num_,
          this->width_,
          this->height_,
          Util::deg_to_rad(this->alpha_),
          Util::deg_to_rad(this->beta_),
          this->scope_,
          this->speed_,
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

  if (current.stop != ctrl.stop_                      ||
      !Util::floats_same(current.alpha, truth.alpha_) ||
      !Util::floats_same(current.beta,  truth.beta_)  ||
      !Util::floats_same(current.scope, truth.scope_) ||
      !Util::floats_same(current.speed, truth.speed_) ||
      !Util::floats_same(current.prad,  truth.prad_))
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
UiState::random(Log& log)
{
  this->alpha_ = Util::distribute<float>(-180.0f, 180.0f);
  this->beta_ = Util::distribute<float>(-180.0f, 180.0f);
  this->scope_ = Util::distribute<float>(1.0f, 96.0f);
  this->speed_ = Util::distribute<float>(1.0f, 32.0f);
  log.add(Attn::O,
          "Random: a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_)
          + ", v=" + std::to_string(this->scope_)
          + ", s=" + std::to_string(this->speed_));
  this->deceive();
}


void
UiState::coloring(Log& log, Coloring scheme)
{
  std::string message;
  if      (Coloring::Normal    == scheme) { message = "normal"; }
  else if (Coloring::Cluster   == scheme) { message = "cluster-based"; }
  else if (Coloring::Density10 == scheme) { message = "density threshold 10"; }
  else if (Coloring::Density15 == scheme) { message = "density threshold 15"; }
  else if (Coloring::Density20 == scheme) { message = "density threshold 20"; }
  else if (Coloring::Density25 == scheme) { message = "density threshold 25"; }
  else if (Coloring::Density30 == scheme) { message = "density threshold 30"; }
  this->coloring_ = scheme;
  this->ctrl_.coloring(scheme);
  log.add(Attn::O, "Color scheme to " + message + ".");
  this->deceive();
}


void
UiState::pattern(Log& log)
{
  std::string message;
  int pattern = this->pattern_;
  if (0 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = 17.0f;
    message = "Lifelike structures 1";
  } else if (1 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = -7.0f;
    message = "Moving structures";
  } else if (2 == pattern) {
    this->alpha_ = 180.0f; this->beta_ = -15.0f;
    message = "Clean cow pattern";
  } else if (3 == pattern) {
    this->alpha_ = 90.0f; this->beta_ = -21.0f;
    message = "Chaos w/ random aggr. 1";
  } else if (4 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -10.0f;
    message = "Fingerprint pattern";
  } else if (5 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -41.0f;
    message = "Chaos w/ random aggr. 2";
  } else if (6 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = -25.0f;
    message = "Untidy cow pattern";
  } else if (7 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = -48.0f;
    message = "Chaos w/ random aggr. 3";
  } else if (8 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = 5.0f;
    message = "Regular pattern";
  } else if (9 == pattern) {
    this->alpha_ = -159.0f; this->beta_ = 15.0f;
    message = "Lifelike structures 2";
  } else if (10 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 1.0f;
    message = "Stable cluster pattern";
  } else if (11 == pattern) {
    this->alpha_ = -180.0f; this->beta_ = 58.0f;
    message = "Chaotic pattern 1";
  } else if (12 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 40.0f;
    message = "Chaotic pattern 2";
  } else if (13 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 8.0f;
    message = "Cells & moving cluster";
  } else if (14 == pattern) {
    this->alpha_ = 0.0f; this->beta_ = 0.0f;
    message = "Chaotic pattern 3";
  } else if (15 == pattern) {
    this->alpha_ = 45.0f; this->beta_ = 4.0f;
    message = "Stable rings";
  }
  log.add(Attn::O,
          message
          + ": a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_));
  this->deceive();
}


bool
UiState::save(const std::string& path)
{
  return this->ctrl_.save(path);
}


bool
UiState::load(const std::string& path)
{
  Stative loaded = this->ctrl_.load(path);
  if (-1 == loaded.num) {
    return false;
  }
  this->stop_     = loaded.stop;
  this->num_      = loaded.num;
  this->width_    = loaded.width;
  this->height_   = loaded.height;
  this->alpha_    = Util::rad_to_deg(loaded.alpha);
  this->beta_     = Util::rad_to_deg(loaded.beta);
  this->scope_    = loaded.scope;
  this->speed_    = loaded.speed;
  this->prad_     = loaded.prad;
  this->coloring_ = (Coloring)loaded.coloring;
  return true;
}

