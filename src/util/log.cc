#include "log.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Log::Log()
{
  this->limit_ = 128;
}


void
Log::Add(Attn attn, const std::string &message)
{
  if (this->limit_ <= this->messages_.size())
  {
    this->messages_.pop_back();
  }
  std::string m = message;
  if      (attn == Attn::E)   { m = "Error: " + m; }
  else if (attn == Attn::Ecl) { m = "Error (cl): " + m; }
  else if (attn == Attn::Egl) { m = "Error (cl): " + m; }
  this->messages_.push_front(std::pair<Attn,std::string>(attn, m));
}

