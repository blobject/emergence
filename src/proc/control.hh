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
 private:
  Log   &log_;
  Proc  &proc_;
  State &state_;

 public:
  long long stop_; // # iterations until system processing stops
  bool      quit_;

  Control(Log &log, State &state, Proc &proc, const std::string &init);

  void    AttachToState(Observer &observer);
  void    DetachFromState(Observer &observer);
  void    AttachToProc(Observer &observer);
  void    DetachFromProc(Observer &observer);
  void    Next();
  void    Pause(bool yes);
  void    Done();
  void    Quit();
  bool    ClGood();
  //inline State &get_state() const { return this->state_; }
  State   &GetState();
  int     GetNum();
  bool    Different(Stative &gui);
  bool    Change(Stative &gui);
  Stative Load(const std::string &path);
  bool    Save(const std::string &path);
  bool    LoadFromFile(const std::string &path);
  bool    SaveToFile(const std::string &path);
};

