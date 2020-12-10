#include "log.hh"


Log::Log(unsigned int limit)
  : limit_(limit)
{}


// Add: Push a new message into the log.

void
Log::Add(Attn attn, const std::string &message)
{
  if (this->limit_ <= this->messages_.size())
  {
    this->messages_.pop_back();
  }
  std::string m = message;
  if      (attn == Attn::E)   { m = "Error: " + m; }
  else if (attn == Attn::Ecl) { m = "Error(cl): " + m; }
  else if (attn == Attn::Egl) { m = "Error(gl): " + m; }
  this->messages_.push_front(std::pair<Attn,std::string>(attn, m));

  this->Notify(Topic::NewMessage); // Headless reacts
}
