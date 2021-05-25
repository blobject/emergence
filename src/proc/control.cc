#include "control.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <chrono>
#include <fstream>
#include <sstream>


Control::Control(Log& log, State& state, Proc& proc, ExpControl& expctrl,
                 Exp& exp, const std::string& init_path, bool pause)
  : exp_(exp), expctrl_(expctrl), log_(log), proc_(proc), state_(state),
    paused_(pause)
{
  this->pid_ = static_cast<int>(getpid());
  this->duration_ = -1;
  this->tick_ = 0;
  this->step_ = false;
  this->quit_ = false;
  if (!init_path.empty()) {
    this->load(init_path);
  }
  expctrl.control(*this);
  this->countdown_ = this->duration_;
  this->gui_change_ = false;
  this->profile_ago_ = std::chrono::steady_clock::now();
  this->profile_fps_ = 0;
  this->profile_count_ = 0;
  //this->profile_max_ = 10;
  this->profile_max_ = 120;

  log.add(Attn::O, "Started control module.");
}


void
Control::profile()
{
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  ++this->profile_fps_;
  int since = std::chrono::duration_cast<std::chrono::microseconds>(
    now - this->profile_ago_).count();
  if (1000000 > since) {
    this->profile_last_ = now;
    return;
  }
  if (this->profile_max_ <= this->profile_count_) {
    this->quit();
    return;
  }
  int last = std::chrono::duration_cast<std::chrono::microseconds>(
    now - this->profile_last_).count();
  std::cout << this->profile_fps_ + static_cast<float>(since - 1000000) / last
            << "\n";
  this->profile_fps_ = 0;
  this->profile_ago_ = now;
  ++this->profile_count_;
  /**
  if (this->profile_max_ <= this->profile_count_) {
    this->profile_count_ = 0;
    State& state = this->state_;
    float dpe = 0.002f + static_cast<float>(state.num_) / state.width_ / state.height_;
    unsigned int w = state.width_ + 50;
    unsigned int h = state.height_ + 50;
    int n = static_cast<int>(w * h * dpe);
    float v = state.scope_ + 2.0f;
    if (1500 < w) {
      this->quit();
      return;
    }
    std::cout << "\nw" << w << ",h" << h << ",d" << dpe << ",v" << v << ":";
    Stative stative = {
      -1, n, w, h,
      state.alpha_, state.beta_,
      v, v * 0.26f,
      state.speed_, 0.0f, state.prad_, state.coloring_
    };
    this->duration_ = 1;
    this->change(stative, true);
    this->gui_change_ = true; // let UiState reflect true State
    return;
  }
  int last = std::chrono::duration_cast<std::chrono::microseconds>(
    now - this->profile_last_).count();
  std::cout << " "
            << this->profile_fps_ + static_cast<float>(since - 1000000) / last;
  this->profile_fps_ = 0;
  this->profile_ago_ = now;
  ++this->profile_count_;
  //*/
}


void
Control::next()
{
  //this->profile();

  // when countdown drops to 0, Proc should exclaim completion
  Proc& proc = this->proc_;
  long long countdown = this->countdown_;

  if (0 == countdown) {
    this->done();
  }

  if (this->paused_ && !this->step_) {
    // TODO: separate concerns
    proc.notify(Issue::ProcNextDone); // Views react
    return;
  }

  Exp& exp = this->exp_;

  exp.type();
  proc.next();
  this->expctrl_.next(exp, *this);
  this->step_ = false;
  ++this->tick_;
  if (-1 >= countdown) {
    return;
  }
  --this->countdown_;
}


void
Control::attach_to_state(Observer& observer)
{
  this->state_.attach(observer);
}


void
Control::detach_from_state(Observer& observer)
{
  this->state_.detach(observer);
}


void
Control::attach_to_proc(Observer& observer)
{
  this->proc_.attach(observer);
}


void
Control::detach_from_proc(Observer& observer)
{
  this->proc_.detach(observer);
}


void
Control::change(Stative& input, bool respawn)
{
  this->state_.change(input, respawn);
  long long duration = input.duration;
  if (duration != this->duration_) {
    this->duration_ = duration;
    this->countdown_ = duration;
    this->tick_ = 0;
  }
}


Stative
Control::load(const std::string& path)
{
  State& state = this->state_;
  int num;

  if (this->load_file(path)) {
    // truth has changed
    this->log_.add(Attn::O, "Loaded state from '" + path + "'.");
    num = state.num_;
  } else {
    this->log_.add(Attn::E, "Could not load from file '" + path + "'.");
    num = -1;
  }

  return {this->countdown_, num, state.width_, state.height_,
          state.alpha_, state.beta_, state.scope_, state.ascope_,
          state.speed_, state.noise_, state.prad_, state.coloring_};
}


bool
Control::save(const std::string& path)
{
  if (this->save_file(path)) {
    this->log_.add(Attn::O, "Saved state to '" + path + "'.");
    return true;
  }
  this->log_.add(Attn::E, "Could not save to file '" + path + "'.");
  return false;
}


bool
Control::load_file(const std::string& path)
{
  State& truth = this->state_;
  std::ifstream stream(path);
  if (!stream) {
    return false;
  }
  std::string line;
  std::istringstream linestream;
  std::getline(stream, line);
  if (!line.empty()) {
    long long duration;
    unsigned int width;
    unsigned int height;
    float alpha;
    float beta;
    float scope;
    float ascope;
    float speed;
    float noise;
    float prad;
    linestream = std::istringstream(line);
    // on read failure, the parameters are left unchanged
    if (linestream >> duration) this->duration_ = duration;
    if (linestream >> width)    truth.width_    = width;
    if (linestream >> height)   truth.height_   = height;
    if (linestream >> alpha)    truth.alpha_    = Util::deg_to_rad(alpha);
    if (linestream >> beta)     truth.beta_     = Util::deg_to_rad(beta);
    if (linestream >> scope)    truth.scope_    = scope;
    if (linestream >> ascope)   truth.ascope_   = ascope;
    if (linestream >> speed)    truth.speed_    = speed;
    if (linestream >> noise)    truth.noise_    = Util::deg_to_rad(noise);
    if (linestream >> prad)     truth.prad_     = prad;
  }
  this->countdown_ = this->duration_;
  this->tick_ = 0;

  float w = static_cast<float>(truth.width_);
  float h = static_cast<float>(truth.height_);
  unsigned int n_stride = truth.n_stride_;
  unsigned int i;
  float px;
  float py;
  float pf;
  float pf_rad;
  unsigned int count = 0;
  truth.clear();
  while (std::getline(stream, line)) {
    if (line.empty()) {
      continue;
    }
    linestream = std::istringstream(line);
    linestream >> i; // ignore the leading particle index
    if (!(linestream >> px)) { px = Util::distr(0.0f, w); }
    truth.px_.push_back(px);
    if (!(linestream >> py)) { py = Util::distr(0.0f, h); }
    truth.py_.push_back(py);
    if (!(linestream >> pf)) { pf = Util::distr(0.0f, 360.0f); }
    pf_rad = Util::deg_to_rad(pf);
    truth.pf_.push_back(pf_rad);
    truth.pc_.push_back(cosf(pf_rad));
    truth.ps_.push_back(sinf(pf_rad));
    truth.pn_.push_back(0);
    truth.pl_.push_back(0);
    truth.pr_.push_back(0);
    truth.pan_.push_back(0);
    for (int j = 0; j < n_stride; ++j) { truth.pls_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { truth.prs_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { truth.pld_.push_back(-1.0f); }
    for (int j = 0; j < n_stride; ++j) { truth.prd_.push_back(-1.0f); }
    truth.pt_.push_back(Type::None);
    truth.gcol_.push_back(0);
    truth.grow_.push_back(0);
    truth.xr_.push_back(1.0f);
    truth.xg_.push_back(1.0f);
    truth.xb_.push_back(1.0f);
    truth.xa_.push_back(0.5f);
    ++count;
  }
  truth.num_ = count;
  if (0 == count) {
    truth.num_ = 1000;
    truth.spawn();
  }
  // TODO: bug when random -> load (change() doesn't help)
  return true;
}


bool
Control::save_file(const std::string& path)
{
  State& truth = this->state_;
  std::ofstream stream(path);
  if (!stream) {
    return false;
  }
  stream << this->duration_ << ' '
         << truth.width_ << ' '
         << truth.height_ << ' '
         << Util::rad_to_deg(truth.alpha_) << ' '
         << Util::rad_to_deg(truth.beta_) << ' '
         << truth.scope_ << ' '
         << truth.ascope_ << ' '
         << truth.speed_ << ' '
         << Util::rad_to_deg(truth.noise_) << ' '
         << truth.prad_ << '\n';
  for (int i = 0; i < truth.num_; ++i) {
    stream << i << ' '
           << truth.px_[i] << ' '
           << truth.py_[i] << ' '
           << Util::rad_to_deg(truth.pf_[i]) << '\n';
  }
  stream.close();
  return true;
}


void
Control::pause(bool yesno)
{
  this->paused_ = yesno;
}


void
Control::done()
{
  this->pause(true);
  this->proc_.done();
}


void
Control::quit()
{
  this->quit_ = true;
}


bool
Control::cl_good() const
{

#if 1 == CL_ENABLED

  return this->proc_.cl_good_;

#endif /* CL_ENABLED */

  return false;

}


void
Control::reset_exp()
{
  this->exp_.reset_exp();
}


std::string
Control::color(Coloring scheme)
{
  this->exp_.color(scheme);

  std::string which;
  if      (Coloring::Original  == scheme) { which = "original"; }
  else if (Coloring::Dynamic   == scheme) { which = "dynamic"; }
  else if (Coloring::Cluster   == scheme) { which = "clustered"; }
  else if (Coloring::Inspect   == scheme) { which = "inspected"; }
  else if (Coloring::Density10 == scheme) { which = "density 10"; }
  else if (Coloring::Density15 == scheme) { which = "density 15"; }
  else if (Coloring::Density20 == scheme) { which = "density 20"; }
  else if (Coloring::Density25 == scheme) { which = "density 25"; }
  else if (Coloring::Density30 == scheme) { which = "density 30"; }
  else if (Coloring::Density35 == scheme) { which = "density 35"; }
  else if (Coloring::Density40 == scheme) { which = "density 40"; }
  std::ostringstream message;
  message << "Coloring set to " << which << ".";

  return message.str();
}


std::string
Control::highlight(std::vector<unsigned int>& particles) {
  this->exp_.highlight(particles);

  unsigned int size = particles.size();
  std::ostringstream message;
  message << "Highlighted " << size << " particle" << (size == 1 ? "" : "s")
          << ".";

  return message.str();
}


std::string
Control::cluster(float radius, unsigned int minpts)
{
  Exp& exp = this->exp_;
  exp.cluster(radius, minpts);

  float num = static_cast<float>(this->state_.num_);
  unsigned int num_cores = exp.cores_.size();
  unsigned int num_vague = exp.vague_.size();

  std::ostringstream message;
  message << std::fixed << std::setprecision(2)
          << exp.clusters_.size() << " Clusters found.\n"
          << "  cells: " << exp.cell_clusters_.size() << "\n"
          << "  mature spores: " << exp.spore_clusters_.size() << "\n"
          << "  cores: " << num_cores << " (" << num_cores * 100 / num << "%)\n"
          << "  vague: " << num_vague << " (" << num_vague * 100 / num << "%)\n"
          << "  noise: " << num - num_cores - num_vague;

  return message.str();
}

std::string
Control::inject(Type type, bool greater)
{
  State& state = this->state_;
  Exp& exp = this->exp_;
  unsigned int size = exp.sprites_[type].size();
  std::ostringstream message;

  exp.inject(type, greater);
  state.notify(Issue::StateChanged); // Canvas reacts TODO

  message.precision(4);
  message << "Injected ";
  if (greater) {
    message << "greater ";
  }
  message << state.type_name(type) << ".\n"
          << "  # particles: " << size << "\n"
          << "  placement: index=" << state.num_ - size
          << ", x=" << exp.sprite_x_ << ", y=" << exp.sprite_y_;

  return message.str();
}

