#include "util.hh"


// Attach: Attach an observer

void
Subject::Attach(Observer &observer)
{
  this->observers.push_back(&observer);
}


// Detach: Detach an observer

void
Subject::Detach(Observer &observer)
{
  this->observers.erase(std::remove(this->observers.begin(),
                                    this->observers.end(),
                                    &observer));
}


// Notify: Perform an action on modification

void
Subject::Notify()
{
  for (auto* observer : this->observers)
  {
    observer->React(*this);
  }
}


