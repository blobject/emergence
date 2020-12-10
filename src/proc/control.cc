#include <fstream>
#include <sstream>

#include "control.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Control::Control(Log &log, State &state, Proc &proc,
                 const std::string &init_path)
  : log_(log), proc_(proc), state_(state)
{
  this->stop_ = -1;
  this->quit_ = false;
  if (! init_path.empty())
  {
    this->Load(init_path);
  }
}


// Observer pattern helpers for at/de-taching Views to State/Proc.

void
Control::AttachToState(Observer &observer)
{
  this->state_.Attach(observer);
}


void
Control::DetachFromState(Observer &observer)
{
  this->state_.Detach(observer);
}


void
Control::AttachToProc(Observer &observer)
{
  this->proc_.Attach(observer);
}


void
Control::DetachFromProc(Observer &observer)
{
  this->proc_.Detach(observer);
}


// Next: Call Proc.Next() while handling paused state and remaining ticks.

void
Control::Next()
{
  // when stop drops to 0, Proc should exclaim completion
  Proc &proc = this->proc_;
  long long stop = this->stop_;
  if (0 == stop)
  {
    this->Done();
  }
  proc.Next();
  // decrement tick count ("stop") unless -1 (eternal) or system paused
  if (! proc.paused_ && -1 < stop)
  {
    this->stop_ = stop - 1;
  }
}


// Pause: Do not let the system perform its action.

void
Control::Pause(bool yes)
{
  this->proc_.paused_ = yes;
}


// Done: Thin wrapper around Proc.Done().

void
Control::Done()
{
  this->proc_.Done();
}


// Quit: Stops the main() loop.

void
Control::Quit()
{
  this->quit_ = true;
}


// ClGood(): Thin wrapper around Cl.Good().

bool
Control::ClGood()
{
  return this->proc_.cl_good_;
}


// GetState(): Return a mutable reference to State.

State&
Control::GetState()
{
  return this->state_;
}


// GetNum(): Return the number of particles.

int
Control::GetNum()
{
  return this->state_.num_;
}


// Different: Is GuiState different from State (source of truth)?

bool
Control::Different(Stative &gui)
{
  State &truth = this->state_;
  return (gui.stop   != this->stop_   ||
          gui.num    != truth.num_    ||
          gui.width  != truth.width_  ||
          gui.height != truth.height_ ||
          Util::RoundF(gui.alpha) != Util::RoundF(truth.alpha_) ||
          Util::RoundF(gui.beta)  != Util::RoundF(truth.beta_)  ||
          Util::RoundF(gui.scope) != Util::RoundF(truth.scope_) ||
          Util::RoundF(gui.speed) != Util::RoundF(truth.speed_) ||
          gui.colors != truth.colors_);
}


// Change: Change State parameters.

bool
Control::Change(Stative &gui)
{
  return this->state_.Change(gui);
}


// Save: Record the current State.

bool
Control::Save(const std::string &path)
{
  if (! this->SaveToFile(path))
  {
    this->log_.Add(Attn::E, "Could not save to file '" + path + "'.");
    return false;
  }
  this->log_.Add(Attn::O, "Saved state to '" + path + "'.");
  return true;
}


// Load: Patch in an initialising State.

Stative
Control::Load(const std::string &path)
{
  State &state = this->state_;
  int num = state.num_;
  if (! this->LoadFromFile(path))
  {
    this->log_.Add(Attn::E, "Could not load from file '" + path + "'.");
    num = -1;
  }
  this->log_.Add(Attn::O, "Loaded state from '" + path + "'.");
  Stative stative = {this->stop_,
                     num,
                     state.width_,
                     state.height_,
                     state.alpha_,
                     state.beta_,
                     state.scope_,
                     state.speed_,
                     state.colors_};
  return stative;
}


/* State file format
 *
 * - Delimited by horizontal space (' ') and vertical space ('\n')
 * - First line contains non-particle-specific data
 * - Second line and onwards contain particle data
 * - Namely:
 *
 * STOP WIDTH HEIGHT ALPHA BETA SCOPE SPEED
 * 0 X_0 Y_0 PHI_0 RAD_0
 * 1 X_1 Y_1 PHI_1 RAD_1
 * ...
 */

// LoadFromFile: Parse a file containing an initialising State.

bool
Control::LoadFromFile(const std::string &path)
{
  State &truth = this->state_;
  std::ifstream stream(path);
  if (! stream)
  {
    return false;
  }
  std::string line;
  std::istringstream linestream;
  std::getline(stream, line);
  if (! line.empty())
  {
    long long stop;
    unsigned int width;
    unsigned int height;
    float alpha;
    float beta;
    float scope;
    float speed;
    linestream = std::istringstream(line);
    // on read failure, the parameters are left unchanged
    if (linestream >> stop) this->stop_ = stop;
    if (linestream >> width) truth.width_ = width;
    if (linestream >> height) truth.height_ = height;
    if (linestream >> alpha) truth.alpha_ = Util::DegToRad(alpha);
    if (linestream >> beta) truth.beta_ = Util::DegToRad(beta);
    if (linestream >> scope) truth.scope_ = scope;
    if (linestream >> speed) truth.speed_ = speed;
  }
  unsigned int i;
  float px;
  float py;
  float pf;
  float prad;
  unsigned int count = 0;
  truth.Clear();
  while (std::getline(stream, line))
  {
    if (line.empty())
    {
      continue;
    }
    linestream = std::istringstream(line);
    linestream >> i; // ignore the leading particle index
    if (! (linestream >> px))
    {
      px = Util::Distribute<float>(0.0f, static_cast<float>(truth.width_));
    }
    truth.px_.push_back(px);
    if (! (linestream >> py))
    {
      py = Util::Distribute<float>(0.0f, static_cast<float>(truth.height_));
    }
    truth.py_.push_back(py);
    if (! (linestream >> pf))
    {
      pf = Util::Distribute<float>(0.0f, TAU);
    }
    truth.pf_.push_back(pf);
    if (! (linestream >> prad)) { prad = 2.0f; }
    truth.prad_.push_back(prad);
    truth.pc_.push_back(cosf(truth.pf_.back()));
    truth.ps_.push_back(sinf(truth.pf_.back()));
    truth.pn_.push_back(0);
    truth.pl_.push_back(0);
    truth.pr_.push_back(0);
    truth.pgcol_.push_back(0);
    truth.pgrow_.push_back(0);
    ++count;
  }
  truth.num_ = count;
  if (count == 0)
  {
    truth.num_ = 1000;
    truth.Spawn();
  }
  // TODO: bug when random -> load (Change() doesn't help)
  return true;
}


// SaveToFile: Write the current State to a file.

bool
Control::SaveToFile(const std::string &path)
{
  State &truth = this->state_;
  std::ofstream stream(path);
  if (! stream)
  {
    return false;
  }
  stream << this->stop_ << ' '
         << truth.width_ << ' '
         << truth.height_ << ' '
         << Util::RadToDeg(truth.alpha_) << ' '
         << Util::RadToDeg(truth.beta_) << ' '
         << truth.scope_ << ' '
         << truth.speed_ << '\n';
  for (unsigned int i = 0; i < truth.num_; ++i)
  {
    stream << i << ' '
           << truth.px_[i] << ' '
           << truth.py_[i] << ' '
           << truth.pf_[i] << ' '
           << truth.prad_[i] << '\n';
  }
  stream.close();
  return true;
}

