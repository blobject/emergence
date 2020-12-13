#include "control.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <fstream>
#include <sstream>


Control::Control(Log& log, State& state, Proc& proc,
                 const std::string& init_path)
    : log_(log), proc_(proc), state_(state)
{
    this->stop_ = -1;
    this->quit_ = false;
    if (!init_path.empty()) {
        this->load(init_path);
    }
    this->start_ = this->stop_;
}


// Observer pattern helpers for at/de-taching Views to State/Proc.

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


// next: Call Proc::next() while handling paused state and remaining ticks.

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
    // decrement tick count ("stop") unless -1 (eternal) or system paused
    if (!proc.paused_ && -1 < stop) {
        this->stop_ = stop - 1;
    }
}


// pause: Do not let the system perform its action.

void
Control::pause(bool yesno)
{
    this->proc_.paused_ = yesno;
}


// done: Thin wrapper around Proc::done().

void
Control::done() const
{
    this->proc_.done();
}


// quit: Stops the main() loop.

void
Control::quit()
{
    this->quit_ = true;
}


// cl_good(): Thin wrapper around Cl::good().

bool
Control::cl_good() const
{
    return this->proc_.cl_good_;
}


// get_state(): Return a mutable reference to State.

State&
Control::get_state() const
{
    return this->state_;
}


// get_num(): Return the number of particles.

int
Control::get_num() const
{
    return this->state_.num_;
}


// different: Is input state different from source-of-truth state?

bool
Control::different(Stative& input)
{
  State& truth = this->state_;
  return (input.stop   != this->stop_   ||
          input.num    != truth.num_    ||
          input.width  != truth.width_  ||
          input.height != truth.height_ ||
          Util::round_float(input.alpha) != Util::round_float(truth.alpha_) ||
          Util::round_float(input.beta)  != Util::round_float(truth.beta_)  ||
          Util::round_float(input.scope) != Util::round_float(truth.scope_) ||
          Util::round_float(input.speed) != Util::round_float(truth.speed_) ||
          input.colors != truth.colors_);
}


// change: Change State parameters.

bool
Control::change(Stative& input) const
{
    return this->state_.change(input);
}


// save: Record the current State.

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


// load: Patch in an initialising State.

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
                      state.colors_};
    return loaded;
}


/* State file format
 *
 * - Delimited by horizontal space (' ') and vertical space ('\n')
 * - First line contains non-particle-specific data
 * - Second line and onwards contain particle data
 * - Namely:
 *
 * START WIDTH HEIGHT ALPHA BETA SCOPE SPEED
 * 0 X_0 Y_0 PHI_0 RAD_0
 * 1 X_1 Y_1 PHI_1 RAD_1
 * ...
 */

// load_file: Parse a file containing an initialising State.

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
        linestream = std::istringstream(line);
        // on read failure, the parameters are left unchanged
        if (linestream >> stop)   this->stop_   = stop;
        this->start_ = this->stop_;
        if (linestream >> width)  truth.width_  = width;
        if (linestream >> height) truth.height_ = height;
        if (linestream >> alpha)  truth.alpha_  = Util::deg_to_rad(alpha);
        if (linestream >> beta)   truth.beta_   = Util::deg_to_rad(beta);
        if (linestream >> scope)  truth.scope_  = scope;
        if (linestream >> speed)  truth.speed_  = speed;
    }
    unsigned int i;
    float px;
    float py;
    float pf;
    float prad;
    unsigned int count = 0;
    truth.clear();
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        linestream = std::istringstream(line);
        linestream >> i; // ignore the leading particle index
        if (!(linestream >> px)) {
            px = Util::distribute<float>(0.0f,
                                         static_cast<float>(truth.width_));
        }
        truth.px_.push_back(px);
        if (!(linestream >> py)) {
            py = Util::distribute<float>(0.0f,
                                         static_cast<float>(truth.height_));
        }
        truth.py_.push_back(py);
        if (!(linestream >> pf)) {
            pf = Util::distribute<float>(0.0f, TAU);
        }
        truth.pf_.push_back(pf);
        if (!(linestream >> prad)) { prad = 2.0f; }
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
    if (count == 0) {
        truth.num_ = 1000;
        truth.spawn();
    }
    // TODO: bug when random -> load (change() doesn't help)
    return true;
}


// save_file: Write the current State to a file.

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
           << truth.speed_ << '\n';
    for (unsigned int i = 0; i < truth.num_; ++i) {
        stream << i << ' '
               << truth.px_[i] << ' '
               << truth.py_[i] << ' '
               << truth.pf_[i] << ' '
               << truth.prad_[i] << '\n';
    }
    stream.close();
    return true;
}

