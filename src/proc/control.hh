//===-- proc/control.hh - Control class declaration ------------*- C++ -*-===//
///
/// \file
/// Definition of the Stative struct and declaration of the Control class,
/// which together provide the means for View to mutate/access State, either
/// directly or through Proc.
/// The Stative struct is a data transport between UiState and State.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "proc.hh"
#include "../exp/control.hh"
#include "../exp/exp.hh"
#include <chrono>


enum class Type;
enum class Coloring;
class Exp;
class ExpControl;
class Proc;
class State;

struct Stative
{
  long long    duration; // number of ticks until processing stops
  int          num;      // number of particles (negative for error)
  unsigned int width;    // processable space width
  unsigned int height;   // processable space height
  float        alpha;    // alpha in main formula (radians)
  float        beta;     // beta in main formula (radians)
  float        scope;    // vicinity radius
  float        ascope;   // alternative vicinity radius (for spores)
  float        speed;    // movement multiplier
  float        noise;    // movement noise
  float        prad;     // particle radius
  int          coloring; // particle coloring scheme (int)
};


class Control
{
 public:
  /// constructor: Prepare ticking and the initial state if necessary.
  /// \param log  Log object
  /// \param state  State object
  /// \param proc  Proc object
  /// \param exp  Exp object
  /// \param expctrl  experiment control object
  /// \param init  path to the file containing an initial state
  /// \param pause  whether system should start paused

  Control(Log& log, State& state, Proc& proc, ExpControl& expctrl, Exp& exp,
          const std::string& init, bool pause);

  // next /////////////////////////////////////////////////////////////////////

  /// next(): Handle processing iteration, pausing, ticking, etc.
  void next();

  /// exp_next(): Handle experimentation.
  void exp_next();

  // State ////////////////////////////////////////////////////////////////////

  /// Observer pattern helpers for at/de-taching View to State.
  void attach_to_state(Observer& observer);
  void detach_from_state(Observer& observer);

  /// change(): Change system parameters.
  /// \param input  input system parameters
  /// \param respawn  whether system should respawn
  void change(Stative& input, bool respawn);

  /// load(): Patch in an initialising state.
  /// \param path  path to the file containing an initial state
  /// \returns  loaded system parameters (on failure, Stative.num is -1)
  Stative load(const std::string& path);

  /// save(): Record the current state.
  /// \param path  path to the file to save the current state to
  /// \returns  whether the save was successful
  bool save(const std::string& path);

  /* state file format
   *
   * - Delimited by horizontal space (' ') and vertical space ('\n')
   * - First line contains non-particle-specific data
   * - Second line and onwards contain particle data
   * - Namely:
   *
   * DURATION WIDTH HEIGHT ALPHA BETA SCOPE ASCOPE SPEED NOISE PRAD
   * 0 X0 Y0 PHI0
   * 1 X1 Y1 PHI1
   * ...
   */

  /// load_file(): Parse a file containing an initialising State.
  /// \param path  path to the file containing an initial state
  /// \returns  whether the load was successful
  bool load_file(const std::string& path);

  /// save_file(): Write the current State to a file.
  /// \param path  path to the file to save the current state to
  /// \returns  whether the save was successful
  bool save_file(const std::string& path);

  // Proc /////////////////////////////////////////////////////////////////////

  /// Observer pattern helpers for at/de-taching View to Proc.
  void attach_to_proc(Observer& observer);
  void detach_from_proc(Observer& observer);

  /// pause(): Whether system should perform processing.
  /// \param yesno  whether processing ought to be paused
  void pause(bool yesno);

  /// done(): Thin wrapper around Proc::done().
  void done();

  /// quit(): Stops the main() loop.
  void quit();

  /// cl_good(): Thin wrapper around Cl::good().
  /// \returns  true if OpenCL is enabled
  bool cl_good() const;

  // Exp //////////////////////////////////////////////////////////////////////

  /// reset_exp(): Thin wrapper around Exp::reset().
  void reset_exp();

  /// color(): Thin wrapper around Exp::color().
  /// \param scheme  particle coloring scheme
  /// \returns  coloring result message
  std::string color(Coloring scheme);

  /// highlight(): Thin wrapper around Exp::highlight().
  /// \param particles  list of particle indices to highlight
  /// \returns  coloring result message
  std::string highlight(std::vector<unsigned int>& particles);

  /// cluster(): Thin wrapper around Exp::cluster().
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  /// \returns  analysis result message
  std::string cluster(float radius, unsigned int minpts);

  /// inject(): Thin wrapper around Exp::inject().
  /// \param type  particle cluster type to be injected
  /// \param greater  whether the greater scope is to be injected
  /// \returns  analysis result message
  std::string inject(Type type, bool greater);

  // members //////////////////////////////////////////////////////////////////

  Exp&        exp_;
  ExpControl& expctrl_;
  State&      state_;

  int           pid_;       // linux process id
  unsigned long tick_;      // number of ticks (time steps, frames) so far
  long long     countdown_; // number of ticks remaining
  long long     duration_;  // initial number of ticks of processing
  bool          paused_;    // whether processing is paused
  bool          step_;      // whether to process one frame at a time
  bool          quit_;      // whether processing ought to stop
  bool          gui_change_;
  float         dpe_;

 private:
  /// profile(): Print the framerate.
  void profile();

  Log&     log_;
  Proc&    proc_;
  std::chrono::steady_clock::time_point profile_ago_;
  std::chrono::steady_clock::time_point profile_last_;
  unsigned int profile_fps_;
  unsigned int profile_count_;
  unsigned int profile_max_;
};

