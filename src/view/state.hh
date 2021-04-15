//===-- view/state.hh - UiState class definition ---------------*- C++ -*-===//
///
/// \file
/// Declaration of the UiState lass, which is an intermediary State parameter
/// storage before user input truly modifies State.
/// UiState communicates with State via the Control class.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../proc/control.hh"
#include "../util/util.hh"


class UiState
{
public:
  /// constructor: Initialise UiState with Control-provided State parameters.
  /// \param ctrl  Control object
  UiState(Control& ctrl);

  /// current(): Get current UI's state.
  /// \returns  snapshot of current UI's state
  Stative current() const;

  /// untrue(): Ask Control whether UI's parameters are different from
  ///           State's.
  /// \returns  0 if UI's state is the same as true State
  ///           1 if UI's state is non-respawn-different from true State
  ///           -1 if UI's state is respawn-different from true State
  int untrue() const;

  /// deceive(): Change true State parameters.
  /// \param respawn  whether system should respawn
  void deceive(bool respawn = false) const;

  /// random(): Randomise true State parameters.
  /// \returns  parameter preset message
  std::string random();

  /// pattern(): Apply pattern parameters (ALPHA & BETA) to true State.
  /// \returns  parameter preset message
  std::string pattern();

  /// save(): Thin wrapper around Control.Save().
  /// \param path  string of path to the save file
  bool save(const std::string& path);

  /// load(): Thin wrapper around Control.load().
  ///         Also update UiState parameters immediately as a difference check
  ///         would be circuitous.
  /// \param path  string of path to the load file
  bool load(const std::string& path);

  Control&     ctrl_;
  long long    stop_;
  int          num_;
  unsigned int width_;
  unsigned int height_;
  float        alpha_; // (degrees)
  float        beta_; // (degrees)
  float        scope_;
  float        ascope_;
  float        speed_;
  float        prad_;
  Coloring     coloring_;
  int          pattern_;
};

