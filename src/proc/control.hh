//===-- control.hh - Control class declaration -----------------*- C++ -*-===//
///
/// \file
/// Definition of the Stative struct and declaration of the Control class,
/// which together provide the means for View to mutate/access State, either
/// directly or through Proc.
/// The Stative struct is a data transport between GuiState and State.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "proc.hh"
#include "../exp/exp.hh"


enum class Coloring;
class Exp;
class Proc;
class State;

struct Stative
{
  long long    stop;     // number of ticks until processing stops
  int          num;      // number of particles (negative for error)
  unsigned int width;    // processable space width
  unsigned int height;   // processable space height
  float        alpha;    // alpha in main formula (radians)
  float        beta;     // beta in main formula (radians)
  float        scope;    // vicinity radius
  float        speed;    // movement multiplier
  float        prad;     // particle radius
  int          coloring; // particle color scheme (int)
};


class Control
{
 public:
  /// constructor: Prepare ticking and the initial state if necessary.
  /// \param log  Log object
  /// \param state  State object
  /// \param proc  Proc object
  /// \param init  path to the file containing an initial state
  Control(Log& log, State& state, Proc& proc, Exp& exp,
          const std::string& init);

  // State ////////////////////////////////////////////////////////////////////

  /// Observer pattern helpers for at/de-taching View to State.
  void attach_to_state(Observer& observer);
  void detach_from_state(Observer& observer);

  /// get_state(): Return a reference to the State.
  /// \returns  reference to the state
  State& get_state() const;

  /// get_num(): Return the number of particles.
  /// \returns  number of particles
  int get_num() const;

  /// get_num(): Return the particle color scheme.
  /// \returns  particle color scheme
  Coloring get_coloring() const;

  /// change(): Change system parameters.
  /// \param input  input system parameters
  /// \param respawn  whether system should respawn
  void change(Stative& input, bool respawn) const;

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
   * START WIDTH HEIGHT ALPHA BETA SCOPE SPEED PRAD
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

  /// next(): Call Proc::next() while handling paused state and the remaining
  ///         ticks.
  void next();

  /// pause(): Whether system should perform processing.
  /// \param yesno  whether processing ought to be paused
  void pause(bool yesno);

  /// done(): Thin wrapper around Proc::done().
  void done() const;

  /// quit(): Stops the main() loop.
  void quit();

  /// cl_good(): Thin wrapper around Cl::good().
  /// \returns  true if OpenCL is enabled
  bool cl_good() const;

  // Exp //////////////////////////////////////////////////////////////////////

  /// reset_exp(): Thin wrapper around Exp::reset().
  void reset_exp();

  /// coloring(): Thin wrapper around Exp::coloring().
  /// \param scheme  particle coloring scheme
  void coloring(Coloring scheme);

  /// cluster(): Thin wrapper around Exp::cluster().
  /// \param radius  DBSCAN neighborhood radius ("epsilon" in literature)
  /// \param minpts  DBSCAN minimum number of neighbors to be considered "core"
  /// \returns  analysis result message
  std::string cluster(float radius, unsigned int minpts);

  /// inject(): Thin wrapper around Exp::inject().
  /// \returns  analysis result message
  std::string inject();

  /// densities(): Thin wrapper around Exp::densities().
  /// \returns  analysis result message
  std::string densities();

  // members //////////////////////////////////////////////////////////////////

  long long start_; // initial number of ticks
  long long stop_;  // number of ticks until processing stops
  bool      quit_;  // whether processing ought to stop

 private:
  Exp&   exp_;
  Log&   log_;
  Proc&  proc_;
  State& state_;
};

