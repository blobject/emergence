#include "control.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <fstream>
#include <sstream>


Control::Control(Log& log, State& state, Proc& proc, Exp& exp,
                 const std::string& init_path)
  : exp_(exp), log_(log), proc_(proc), state_(state)
{
  this->stop_ = -1;
  this->tick_ = 0;
  this->quit_ = false;
  if (!init_path.empty()) {
    this->load(init_path);
  }
  //this->exp_.inject();
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
Control::change(Stative& input, bool respawn) const
{
  this->state_.change(input, respawn);
}


Stative
Control::load(const std::string& path)
{
  State& state = this->state_;
  int num = state.num_;
  if (this->load_file(path)) {
    this->log_.add(Attn::O, "Loaded state from '" + path + "'.");
  } else {
    this->log_.add(Attn::E, "Could not load from file '" + path + "'.");
    num = -1;
  }
  Stative loaded = {this->stop_,
                    num,
                    state.width_,
                    state.height_,
                    state.alpha_,
                    state.beta_,
                    state.scope_,
                    state.speed_,
                    state.prad_,
                    state.coloring_};
  return loaded;
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
      long long stop;
      unsigned int width;
      unsigned int height;
      float alpha;
      float beta;
      float scope;
      float speed;
      float prad;
      linestream = std::istringstream(line);
      // on read failure, the parameters are left unchanged
      if (linestream >> stop)   this->stop_   = stop;
      if (linestream >> width)  truth.width_  = width;
      if (linestream >> height) truth.height_ = height;
      if (linestream >> alpha)  truth.alpha_  = Util::deg_to_rad(alpha);
      if (linestream >> beta)   truth.beta_   = Util::deg_to_rad(beta);
      if (linestream >> scope)  truth.scope_  = scope;
      if (linestream >> speed)  truth.speed_  = speed;
      if (linestream >> prad)   truth.prad_   = prad;
  }
  float w = static_cast<float>(truth.width_);
  float h = static_cast<float>(truth.height_);
  unsigned int i;
  float px;
  float py;
  float pf;
  unsigned int count = 0;
  truth.clear();
  while (std::getline(stream, line)) {
    if (line.empty()) {
      continue;
    }
    linestream = std::istringstream(line);
    linestream >> i; // ignore the leading particle index
    if (!(linestream >> px)) {
      px = Util::dist(0.0f, w);
    }
    truth.px_.push_back(px);
    if (!(linestream >> py)) {
      py = Util::dist(0.0f, h);
    }
    truth.py_.push_back(py);
    if (!(linestream >> pf)) {
      pf = Util::dist(0.0f, TAU);
    }
    truth.pf_.push_back(pf);
    truth.pc_.push_back(cosf(truth.pf_.back()));
    truth.ps_.push_back(sinf(truth.pf_.back()));
    truth.pn_.push_back(0);
    truth.pl_.push_back(0);
    truth.pr_.push_back(0);
    truth.gcol_.push_back(0);
    truth.grow_.push_back(0);
    ++count;
  }
  truth.num_ = count;
  if (count == 0) {
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
  stream << this->stop_ << ' '
         << truth.width_ << ' '
         << truth.height_ << ' '
         << Util::rad_to_deg(truth.alpha_) << ' '
         << Util::rad_to_deg(truth.beta_) << ' '
         << truth.scope_ << ' '
         << truth.speed_ << ' '
         << truth.prad_ << '\n';
  for (int i = 0; i < truth.num_; ++i) {
    stream << i << ' '
           << truth.px_[i] << ' '
           << truth.py_[i] << ' '
           << truth.pf_[i] << '\n';
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
  proc.next();
  this->exp_.type();
  // countdown unless system paused
  if (proc.paused_) {
    return;
  }
  ++this->tick_;
  if (-1 >= stop) {
    return;
  }
  this->stop_ = stop - 1;
}


void
Control::pause(bool yesno)
{
  this->proc_.paused_ = yesno;
}


void
Control::done() const
{
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


void
Control::coloring(Coloring scheme)
{
  this->exp_.coloring(scheme);
}


std::string
Control::cluster(float radius, unsigned int minpts)
{
  Exp& exp = this->exp_;

  exp.cluster(radius, minpts);

  float num = static_cast<float>(this->get_num());
  unsigned int num_cores = exp.cores_.size();
  unsigned int num_vague = exp.vague_.size();

  std::stringstream message;
  message.precision(4);
  message << exp.clusters_.size() << " Clusters found.\n"
          << "cells: " << exp.cell_clusters_ << "\n"
          << "mature spores: " << exp.spore_clusters_ << "\n"
          << "cores: " << num_cores << " (" << num_cores * 100 / num << "%)\n"
          << "vague: " << num_vague << " (" << num_vague * 100 / num << "%)\n"
          << "noise: " << static_cast<int>(num - num_cores - num_vague);

  return message.str();
}

std::string
Control::inject(Type type, float dpe)
{
  Exp& exp = this->exp_;
  exp.inject(type, dpe);
  this->log_.add(Attn::O, "Injected into state.");
  this->state_.notify(Issue::StateChanged); // Canvas reacts
  unsigned int size = exp.sprites_[type].size();

  std::stringstream message;
  message.precision(4);
  message << "Injected " << exp.type_name(type) << ".\n"
          << "index: " << exp.sprite_index_ - size << "\n"
          << "# particles: " << size << "\n"
          << "placement: x=" << exp.sprite_x_ << ", y=" << exp.sprite_y_
          << ", scale=" << exp.sprite_scale_;

  return message.str();
}

