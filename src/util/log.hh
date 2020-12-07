#pragma once

#include <iostream>
#include <string>
#include <deque>


// Attn: Type of log message

enum class Attn { O = 0, E, Ecl, Egl };


// Log: Debug logs

class Log
{
 private:
  unsigned int limit_;

 public:
  std::deque<std::pair<Attn,std::string>> messages_;

  Log();

  void Add(Attn attn, const std::string &message);
};

