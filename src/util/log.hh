#pragma once

#include "../util/observation.hh"
#include <deque>
#include <iostream>
#include <string>


// Attn: Type of log message

enum class Attn { O = 0, E, Ecl, Egl };


// Log: Debug logs

class Log : public Subject
{
  public:
    Log(unsigned int limit);
    void add(Attn attn, const std::string& message);

    std::deque<std::pair<Attn,std::string>> messages_;

  private:
    unsigned int limit_;
};

