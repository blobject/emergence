#include "headless.hh"


Headless::Headless(Log &log, Proc &proc)
  : log_(log), proc_(proc)
{
  log.Attach(*this);
}


Headless::~Headless()
{
  this->log_.Detach(*this);
}


void
Headless::Exec()
{
  Proc &proc = this->proc_;
  while (true)
  {
    proc.Next(); // changes particles' X, Y, F, N, etc.
  }
}


void
Headless::React(Subject &next_log)
{
  std::cout << this->log_.messages_.front().second << std::endl;
}

