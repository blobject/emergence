//===-- exp/control.hh - ExpControl class declaration ----------*- C++ -*-===//
///
/// \file
/// Declaration of the ExpControl class, which modifies other classes in
/// preparation of experiment iterations.
/// ExpControl directly modifies State, Control, and Exp.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "exp.hh"
#include "../proc/control.hh"
#include "../state/state.hh"


class Control;
class Exp;
class State;

class ExpControl
{
 public:
  /// constructor: Initialise experiment control module.
  /// \param log  Log object
  /// \param e  specific experiment being performed
  ExpControl(Log& log, int e);

  /// message(): Print titutlar message about specified experiment.
  void message();

  /// state(): Modify initial habitat parameters according to specified
  ///          experiment.
  /// \param s  State object
  void state(State& s);

  /// spawn(): Modify initial particles according to specified experiment.
  /// \param s  State object
  /// \returns  true if particles modified
  bool spawn(State& s);

  /// control(): Modify initial Control according to specified experiment.
  /// \param c  Control object
  void control(Control& c);

  /// next(): Iterate Control process according to specified experiment.
  /// \param exp  Exp object
  /// \param c  Control object
  void next(Exp& exp, Control& c);

  /// next*_*(): Control iteration of specified experiment.
  /// \param c  Control object
  /// \returns  (optional) false (or -1) if Emergence should quit.
  void next4_first(Control& c);
  bool next4_iterate(Control& c);
  void next4_change(Control& c);
  void next5_first(Control& c);
  float next5_iterate(Control& c);
  void next5_change(Control& c, float noise);
  bool next6_iterate(Control& c);
  void next6_change(Control& c);

  int experiment_group_; // experiment being perfomed
  int experiment_;       // specific experiment being perfomed

 private:
  Log& log_;
};

