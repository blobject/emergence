//===-- observation.hh - observer pattern -----------------------*- C++ -*-===//
///
/// \file
/// Definition of the Issue enum and the Subject and Observer classes,
/// implementing the observer pattern.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <vector>


// Issue: Type of observer notification.

enum class Issue { StateChanged = 0, ProcNextDone, ProcDone, NewMessage };


class Subject;

class Observer
{
  public:
    virtual ~Observer() = default;
    virtual void react(Issue issue) = 0;
};

class Subject
{
  public:
    std::vector<Observer*> observers_;
    virtual ~Subject() = default;

    inline void
    attach(Observer& observer)
    {
        this->observers_.push_back(&observer);
    }

    inline void
    detach(Observer& observer)
    {
        this->observers_.erase(std::remove(this->observers_.begin(),
                                           this->observers_.end(),
                                           &observer));
    }

    inline void notify(Issue issue) const
    {
        for (Observer* observer : this->observers_) {
            observer->react(issue);
        }
    }
};

