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
  this->start_ = -1;
  this->tick_ = 0;
  this->step_ = false;
  this->quit_ = false;
  if (!init_path.empty()) {
    this->load(init_path);
  }
  int experiment = state.experiment_;
  int experiment_group = state.experiment_group_;
  this->experiment_ = experiment;
  this->experiment_group_ = experiment_group;
  if (experiment) {
    if      (1 == experiment_group) { this->start_ = 150;
      if (14 == experiment)         { this->start_ = 700; } }
    else if (2 == experiment_group) { this->start_ = 100000; }
    else if (3 == experiment_group) { this->start_ = 100;
      if      (30 == experiment) { this->inject(Type::Nutrient,      true); }
      else if (31 == experiment) { this->inject(Type::PrematureSpore,true); }
      else if (32 == experiment) { this->inject(Type::MatureSpore,   true); }
      else if (33 == experiment) { this->inject(Type::Ring,          true); }
      else if (34 == experiment) { this->inject(Type::PrematureCell, true); }
      else if (35 == experiment) { this->inject(Type::TriangleCell,  true); }
      else if (36 == experiment) { this->inject(Type::SquareCell,    true); }
      else if (37 == experiment) { this->inject(Type::PentagonCell,  true); } }
    else if (4 == experiment_group) { this->start_ = 25000;
      // TODO
      if      (40 == experiment) { this->inject(Type::MatureSpore,  false); }
      else if (41 == experiment) { this->inject(Type::TriangleCell, false); }
      //this->gui_change_ = true;
    }
    else if (5 == experiment_group) { this->start_ = 25000;
      if      (50 == experiment) { this->inject(Type::TriangleCell, false); }
      else if (51 == experiment) { this->inject(Type::TriangleCell, false); }
      else if (52 == experiment) { this->inject(Type::TriangleCell, false); }
      // TODO: noise
      else if (53 == experiment) { this->inject(Type::TriangleCell, false); }
      else if (54 == experiment) { this->inject(Type::TriangleCell, false); }
      else if (55 == experiment) { this->inject(Type::TriangleCell, false); } }
    else if (6 == experiment_group) { this->start_ = 500; }
    ++this->start_; // tick one more to gather the very last data set
  }
  this->stop_ = this->start_;
  this->gui_change_ = false; // TODO

  log.add(Attn::O, "Started control module.", true);
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
  return (Coloring)this->state_.coloring_;
}


void
Control::change(Stative& input, bool respawn)
{
  if (input.stop != this->start_) {
    this->start_ = input.stop;
    this->stop_ = input.stop;
    this->tick_ = 0;
  }
  this->state_.change(input, respawn);
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

  return {this->stop_, num, state.width_, state.height_,
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
    long long start;
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
    if (linestream >> start)  this->start_  = start;
    if (linestream >> width)  truth.width_  = width;
    if (linestream >> height) truth.height_ = height;
    if (linestream >> alpha)  truth.alpha_  = Util::deg_to_rad(alpha);
    if (linestream >> beta)   truth.beta_   = Util::deg_to_rad(beta);
    if (linestream >> scope)  truth.scope_  = scope;
    if (linestream >> ascope) truth.ascope_ = ascope;
    if (linestream >> speed)  truth.speed_  = speed;
    if (linestream >> noise)  truth.noise_  = Util::deg_to_rad(noise);
    if (linestream >> prad)   truth.prad_   = prad;
  }
  this->stop_ = this->start_;
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
  stream << this->start_ << ' '
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
Control::next()
{
  // when stop drops to 0, Proc should exclaim completion
  Proc& proc = this->proc_;
  long long stop = this->stop_;

  if (0 == stop) {
    this->done();
  }

  if (this->paused_ && !this->step_) {
    // TODO: separate concerns
    proc.notify(Issue::ProcNextDone); // Views react
    return;
  }

  unsigned long tick = this->tick_;

  this->exp_.type();
  proc.next();
  this->exp_next();
  this->step_ = false;
  this->tick_ = tick + 1;
  if (-1 >= stop) {
    return;
  }
  --this->stop_;
}


void
Control::exp_next()
{
  int experiment = this->experiment_;

  if (!experiment) {
    return;
  }

  Exp& exp = this->exp_;
  unsigned long tick = this->tick_;

  //exp.brief_meta_exp(tick);
  if (experiment) {
    int experiment_group = this->experiment_group_;
    if      (1 == experiment_group) {
      if      (10 == experiment ||
               11 == experiment ||
               12 == experiment ||
               13 == experiment) { exp.do_exp_1a(tick); }
      else if (14 == experiment) { exp.do_exp_1b(tick); } }
    else if (2 == experiment_group) { exp.do_exp_2(tick); }
    else if (3 == experiment_group) { exp.do_exp_3(tick); }
    else if (4 == experiment_group) { if (exp.do_exp_4(tick)) { this->pause(true); }
    /**
    // TODO
    else if (4 == experiment_group) {
      if (exp.do_exp_4(tick)) {
        if (0.1 < this->dpe_) {
          return;
        }
        State& state = this->state_;
        this->dpe_ += 0.001f;
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
        this->change(stative, true);
        if      (40 == experiment) { this->inject(Type::MatureSpore,  false); }
        else if (41 == experiment) { this->inject(Type::TriangleCell, false); }
        this->gui_change_ = true;
        this->pause(false);
      }
    //*/
    }
    else if (5 == experiment_group) { exp.do_exp_5(tick); }
    else if (6 == experiment_group) {
      // TODO
      if (exp.do_exp_6(tick)) {
        State& state = this->state_;
        state.alpha_ = Util::deg_to_rad(Util::rad_to_deg(state.alpha_) + 3.0f);
        this->start_ = 501;
      }
    }
  }
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

