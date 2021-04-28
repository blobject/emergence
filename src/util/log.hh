//===-- util/log.hh - Log class declaration --------------------*- C++ -*-===//
///
/// \file
/// Definition of the Attn enum and declaration of the Log class, which
/// provides a logging mechanism for the entire program.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "../util/observation.hh"
#include <deque>
#include <iostream>
#include <string>


// Attn: Type of log message.

enum class Attn
{
  O = 0,
  E,
  Ecl,
  Egl
};


class Log : public Subject
{
 public:
  /// constructor: Set a limit to the number of messages retained.
  /// \param limit  limit to the number of messages
  /// \param quiet  whether to suppress standard printing
  Log(unsigned int limit, bool quiet = false);

  /// add(): Push a new message into the log.
  /// \param attn  type of message
  /// \param message  message
  void add(Attn attn, const std::string& message);

  // Like add() but also print to Headless stdout. Only used by main(), Proc,
  // and Cl, before Headless gets initialised.
  void add(Attn attn, const std::string& message, bool headless);

  std::deque<std::pair<Attn,std::string>> messages_; // queue of messages
  bool quiet_;         // whether to suppress standard printing

 private:
  unsigned int limit_; // limit to the number of messages
};

