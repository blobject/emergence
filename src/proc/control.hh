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
#include "../util/log.hh"


class Proc;
class State;

struct Stative
{
    long long    stop;   // number of ticks until processing stops
    int          num;    // number of particles (negative for error)
    unsigned int width;  // processable space width
    unsigned int height; // processable space height
    float        alpha;  // alpha in main formula (radians)
    float        beta;   // beta in main formula (radians)
    float        scope;  // vicinity radius
    float        speed;  // movement multiplier
    int          colors; // visualisation colorscheme
};


class Control
{
  public:
    /// constructor: Prepare ticking and the initial state if necessary.
    /// \param log  Log object
    /// \param state  State object
    /// \param proc  Proc object
    /// \param init  path to the file containing an initial state
    Control(Log& log, State& state, Proc& proc, const std::string& init);

    /// Observer pattern helpers for at/de-taching View to State/Proc.
    void attach_to_state(Observer& observer);
    void detach_from_state(Observer& observer);
    void attach_to_proc(Observer& observer);
    void detach_from_proc(Observer& observer);

    /// next(): Call Proc::next() while handling paused state and the remaining
    ///         ticks.
    void    next();

    /// pause(): Do not let the system perform any processing.
    /// \param yesno  whether processing ought to be paused
    void    pause(bool yesno);

    /// done(): Thin wrapper around Proc::done().
    void    done() const;

    /// quit(): Stops the main() loop.
    void    quit();

    /// cl_good(): Thin wrapper around Cl::good().
    /// \returns  true if OpenCL is enabled
    bool    cl_good() const;

    /// get_state(): Return a reference to the State.
    /// \returns  reference to the state
    State& get_state() const;

    /// get_num(): Return the number of particles.
    /// \returns  number of particles
    int get_num() const;

    /// different(): Whether the input system parameters is different from the
    ///              true system parameters?
    /// \param input  input system parameters
    /// \returns  true if input system parameters is different from the true
    ///           system parameters
    bool    different(Stative& input);

    /// change(): Change system parameters.
    /// \param input  input system parameters
    /// \returns  true if particle parameters were respawned
    bool    change(Stative& input) const;

    /// load(): Patch in an initialising state.
    /// \param path  path to the file containing an initial state
    /// \returns  loaded system parameters (on failure, Stative.num is -1)
    Stative load(const std::string& path);

    /// save(): Record the current state.
    /// \param path  path to the file to save the current state to
    /// \returns  whether the save was successful
    bool    save(const std::string& path);

    /* state file format
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

    /// load_file(): Parse a file containing an initialising State.
    /// \param path  path to the file containing an initial state
    /// \returns  whether the load was successful
    bool    load_file(const std::string& path);

    /// save_file(): Write the current State to a file.
    /// \param path  path to the file to save the current state to
    /// \returns  whether the save was successful
    bool    save_file(const std::string& path);

    long long start_; // initial number of ticks
    long long stop_;  // number of ticks until processing stops
    bool      quit_;  // whether processing ought to stop

  private:
    Log&   log_;
    Proc&  proc_;
    State& state_;
};

