#pragma once

#include "proc.hh"
#include "../util/log.hh"


// Mutually include-dependent classes.

class Proc;
class State;


// Stative: Struct for transporting data from GuiState to State.

struct Stative
{
    long long    stop;
    int          num;
    unsigned int width;
    unsigned int height;
    float        alpha;
    float        beta;
    float        scope;
    float        speed;
    int          colors;
};


// Control: What View uses to manipulate State.

class Control
{
  public:
    Control(Log& log, State& state, Proc& proc, const std::string& init);
    void    attach_to_state(Observer& observer);
    void    detach_from_state(Observer& observer);
    void    attach_to_proc(Observer& observer);
    void    detach_from_proc(Observer& observer);
    void    next();
    void    pause(bool yesno);
    void    done() const;
    void    quit();
    bool    cl_good() const;
    //inline State& get_state() const { return this->state_; }
    State&  get_state() const;
    int     get_num() const;
    bool    different(Stative& input);
    bool    change(Stative& input) const;
    Stative load(const std::string& path);
    bool    save(const std::string& path);
    bool    load_file(const std::string& path);
    bool    save_file(const std::string& path);

    long long start_; // initial # iterations
    long long stop_;  // # iterations until system processing stops
    bool      quit_;

  private:
    Log&   log_;
    Proc&  proc_;
    State& state_;
};

