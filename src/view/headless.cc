#include "headless.hh"


Headless::Headless(Log &log, Control &ctrl)
  : log_(log), ctrl_(ctrl)
{
  log.Attach(*this);
  ctrl.AttachToProc(*this);
}


Headless::~Headless()
{
  this->log_.Detach(*this);
  this->ctrl_.DetachFromProc(*this);
}


// Exec: TODO

void
Headless::Exec()
{
}


// React: Headless observes, Log.Add(), Proc.Next(), Proc.Done().

void
Headless::React(Topic topic)
{
  if (Topic::ProcNextDone == topic)
  {
    this->Exec();
  }
  else if (Topic::NewMessage == topic)
  {
    std::cout << this->log_.messages_.front().second << std::endl;
  }
}

