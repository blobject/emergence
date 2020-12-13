#include "headless.hh"


Headless::Headless(Log& log, Control& ctrl)
    : log_(log), ctrl_(ctrl)
{
    log.attach(*this);
    ctrl.attach_to_proc(*this);
}


Headless::~Headless()
{
    this->log_.detach(*this);
    this->ctrl_.detach_from_proc(*this);
}


void
Headless::exec()
{
    // TODO
}


// react: Headless observes, Log::add(), Proc::next(), Proc::done().

void
Headless::react(Topic topic)
{
  if (Topic::ProcNextDone == topic) {
    this->exec();
  } else if (Topic::NewMessage == topic) {
    std::cout << this->log_.messages_.front().second << std::endl;
  }
}

