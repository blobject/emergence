#include "control.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <fstream>
#include <iomanip>
#include <sstream>


Control::Control(Log& log, State& state, Proc& proc, Exp& exp,
                 const std::string& init_path, bool pause)
  : exp_(exp), log_(log), proc_(proc), state_(state), paused_(pause)
{
  this->pid_ = static_cast<int>(getpid());
  this->duration_ = -1;
  this->tick_ = 0;
  this->step_ = false;
  this->quit_ = false;
  if (!init_path.empty()) {
    this->load(init_path);
  }
  int e = state.experiment_;
  int eg = state.experiment_group_;
  this->experiment_ = e;
  this->experiment_group_ = eg;
  if (e) {
    if      (1 == eg) { this->duration_ = 150;
      if      (15 == e) { this->duration_ = 700; } }
    else if (2 == eg) { this->duration_ = 100000; }
    else if (3 == eg) { this->duration_ = 100;
      if      (31 == e) { this->inject(Type::Nutrient,      true); }
      else if (32 == e) { this->inject(Type::PrematureSpore,true); }
      else if (33 == e) { this->inject(Type::MatureSpore,   true); }
      else if (34 == e) { this->inject(Type::Ring,          true); }
      else if (35 == e) { this->inject(Type::PrematureCell, true); }
      else if (36 == e) { this->inject(Type::TriangleCell,  true); }
      else if (37 == e) { this->inject(Type::SquareCell,    true); }
      else if (38 == e) { this->inject(Type::PentagonCell,  true); } }
    else if (4 == eg) { this->duration_ = 25000;
      exp.exp_4_count_ = 1;
      this->dpe_ = static_cast<float>(state.num_)
                   / state.width_ / state.height_;
      // TODO
      if      (41 == e || 43 == e) { this->inject(Type::MatureSpore,  false); }
      else if (42 == e || 44 == e) { this->inject(Type::TriangleCell, false); }
    }
    else if (5 == eg) { this->duration_ = 25000;
      exp.exp_5_count_ = 1;
      this->inject(Type::TriangleCell, false); }
    else if (6 == eg) { this->duration_ = 500; }
    ++this->duration_; // stop after one more tick to gather very last data set
  }
  this->countdown_ = this->duration_;
  this->gui_change_ = false;

  log.add(Attn::O, "Started control module.", true);
}


void
Control::next()
{
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

  this->exp_.type();
  proc.next();
  this->exp_next();
  this->step_ = false;
  ++this->tick_;
  if (-1 >= countdown) {
    return;
  }
  --this->countdown_;
}


void
Control::exp_next()
{
  int e = this->experiment_;

  if (!e) {
    return;
  }

  Exp& exp = this->exp_;
  unsigned long tick = this->tick_;

  //exp.brief_meta_exp(tick);
  int eg = this->experiment_group_;
  if (1 == eg) {
    if (11 == e ||
        12 == e ||
        13 == e ||
        14 == e) { exp.do_exp_1a(tick); return; }
    if (15 == e) { exp.do_exp_1b(tick); return; } return; }
  if (2 == eg) { exp.do_exp_2(tick); return; }
  if (3 == eg) { exp.do_exp_3(tick); return; }
  if (4 == eg) {
    if (1 == tick && 0.0001f > this->dpe_) {
      if (41 == e || 42 == e) {
        if (1 < exp.exp_4_count_) {
          std::cout << "\n";
        }
        std::cout << exp.exp_4_count_ << ":" << std::flush;
      }
    }
    bool done;
    if      (41 == e) { done = exp.do_exp_4a(tick); }
    else if (42 == e) { done = exp.do_exp_4b(tick); }
    else if (43 == e ||
             44 == e) { done = exp.do_exp_4c(tick); }
    if (done) {
      exp.exp_4_est_done_ = 0;
      exp.exp_4_dbscan_done_ = 0;
      this->dpe_ += 0.001f;
      if (0.1001f > this->dpe_ && 40 == e || 41 == e) {
        std::cout << ";";
      }
      if (0.1001f < this->dpe_) {
        ++exp.exp_4_count_;
        this->dpe_ = 0.0f;
      }
      if (10 < exp.exp_4_count_) { // * 10 separate instances
        this->quit_ = true;
        return;
      }
      State& state = this->state_;
      Stative stative = {
        25001,
        static_cast<int>(state.width_ * state.height_ * this->dpe_),
        state.width_,
        state.height_,
        state.alpha_,
        state.beta_,
        state.scope_,
        state.ascope_,
        state.speed_,
        state.noise_,
        state.prad_,
        state.coloring_
      };
      this->duration_ = -1; // for resetting in change()
      this->change(stative, true);
      if      (41 == e || 43 == e) { this->inject(Type::MatureSpore,  false); }
      else if (42 == e || 44 == e) { this->inject(Type::TriangleCell, false); }
      this->gui_change_ = true; // let UiState reflect true State
      return;
    }
    return;
  }
  if (5 == eg) {
    State& state = this->state_;
    if (54 == e || 55 == e || 56 == e) {
      if (1 == tick && 0.0001f > state.noise_) {
        if (1 < exp.exp_5_count_) {
          std::cout << "\n";
        }
        std::cout << exp.exp_5_count_ << ":" << std::flush;
      }
    }
    bool done;
    if      (51 == e || 52 == e || 53 == e) { done = exp.do_exp_5a(tick); }
    else if (54 == e || 55 == e || 56 == e) { done = exp.do_exp_5b(tick); }
    if (done) {
      exp.exp_5_est_done_ = 0;
      exp.exp_5_dbscan_done_ = 0;
      float noise = 0.0f;
      if (51 == e || 52 == e || 53 == e) {
        ++exp.exp_5_count_;
        if (100 < exp.exp_5_count_) { // * 10 separate instances
          this->quit_ = true;
          return;
        }
      } else if (54 == e || 55 == e || 56 == e) {
        if (90.0f <= Util::rad_to_deg(state.noise_)) {
          ++exp.exp_5_count_;
          state.noise_ = Util::deg_to_rad(-5.0f);
        } else {
          std::cout << ";";
        }
        if (100 < exp.exp_5_count_) { // * 10 separate instances
          this->quit_ = true;
          return;
        }
        noise = Util::rad_to_deg(state.noise_) + 5.0f;
      }
      float dpe = 0.03f;
      if      (52 == e || 55 == e) { dpe = 0.035f; }
      else if (53 == e || 56 == e) { dpe = 0.04f; }
      Stative stative = {
        25001,
        static_cast<int>(state.width_ * state.height_ * dpe),
        state.width_,
        state.height_,
        state.alpha_,
        state.beta_,
        state.scope_,
        state.ascope_,
        state.speed_,
        Util::deg_to_rad(noise),
        state.prad_,
        state.coloring_
      };
      this->duration_ = -1; // for resetting in change()
      this->change(stative, true);
      this->inject(Type::TriangleCell, false);
      this->gui_change_ = true; // let UiState reflect true State
      return;
    }
    return;
  }
  if (6 == eg) {
    if (exp.do_exp_6(tick)) {
      State& state = this->state_;
      float alpha = Util::rad_to_deg(state.alpha_);
      float beta = Util::rad_to_deg(state.beta_);
      if (59.5f > beta) {
        state.beta_ = Util::deg_to_rad(beta + 1.0f);
      } else if (179.5f > alpha) {
        state.beta_ = Util::deg_to_rad(-60.0f);
        state.alpha_ = Util::deg_to_rad(alpha + 3.0f);
      } else {
        this->quit_ = true;
        return;
      }
      Stative stative = {
        501,
        1200,
        state.width_,
        state.height_,
        state.alpha_,
        state.beta_,
        state.scope_,
        state.ascope_,
        state.speed_,
        state.noise_,
        state.prad_,
        state.coloring_
      };
      this->duration_ = -1; // for resetting in change()
      this->change(stative, true);
      this->gui_change_ = true; // let UiState reflect true State
      return;
    }
    return;
  }
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


State&
Control::get_state() const
{
  return this->state_;
}


Exp&
Control::get_exp() const
{
  return this->exp_;
}


int
Control::get_num() const
{
  return this->state_.num_;
}


Coloring
Control::get_coloring() const
{
  return static_cast<Coloring>(this->state_.coloring_);
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

  float num = static_cast<float>(this->get_num());
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

