#pragma once

#include <iostream>
#include <string>
#include <deque>

#include "../util/observation.hh"


// Attn: Type of log message

enum class Attn { O = 0, E, Ecl, Egl };


// Log: Debug logs

class Log : public Subject
{
 private:
  unsigned int limit_;

 public:
  std::deque<std::pair<Attn,std::string>> messages_;

  Log(unsigned int limit);

  void Add(Attn attn, const std::string &message);
};

