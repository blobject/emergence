#pragma once

#include <algorithm>
#include <vector>


// Topic: Type of observer notification.

enum class Topic { StateChanged = 0, ProcNextDone, ProcDone, NewMessage };


// Observer design pattern.

class Subject;

class Observer
{
  public:
    virtual ~Observer() = default;
    virtual void react(Topic topic) = 0;
};

class Subject
{
  public:
    std::vector<Observer*> observers_;
    virtual ~Subject() = default;

    // attach: Register an observer.
    inline void
    attach(Observer& observer)
    {
        this->observers_.push_back(&observer);
    }

    // detach: Unregister an observer.
    inline void
    detach(Observer& observer)
    {
        this->observers_.erase(std::remove(this->observers_.begin(),
                                           this->observers_.end(),
                                           &observer));
    }

    // notify: Provoke all observers to react.
    inline void notify(Topic topic) const
    {
        for (Observer* observer : this->observers_) {
            observer->react(topic);
        }
    }
};

