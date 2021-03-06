#include "log.hh"


Log::Log(unsigned int limit, bool quiet /* = false */)
  : limit_(limit), quiet_(quiet)
{}


void
Log::add(Attn attn, const std::string& message, bool stdout)
{
  if (this->limit_ <= this->messages_.size()) {
    this->messages_.pop_back();
  }
  std::string m = message;
  if      (attn == Attn::E)   { m = "Error: "     + m; }
  else if (attn == Attn::Ecl) { m = "Error(cl): " + m; }
  else if (attn == Attn::Egl) { m = "Error(gl): " + m; }
  this->messages_.push_front(std::pair<Attn,std::string>(attn, m));
  this->notify(Issue::NewMessage); // Headless reacts
  std::ostream* stream = &std::cerr;
  if (!this->quiet_ && stdout) {
    if (attn == Attn::O) {
      stream = &std::cout;
    }
    *stream << m << std::endl;
  }
}

