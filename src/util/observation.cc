#include "observation.hh"


// Attach: Register an observer.

void
Subject::Attach(Observer &observer)
{
  this->observers_.push_back(&observer);
}


// Detach: Deregister an observer.

void
Subject::Detach(Observer &observer)
{
  this->observers_.erase(std::remove(this->observers_.begin(),
                                     this->observers_.end(),
                                     &observer));
}


// Notify: Provoke all observers to react.

void
Subject::Notify()
{
  for (Observer* observer : this->observers_)
  {
    observer->React(*this);
  }
}

