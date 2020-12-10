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
  virtual void React(Topic topic) = 0;
};

class Subject
{
 private:

 public:
  std::vector<Observer*> observers_;
  virtual ~Subject() = default;
  void Attach(Observer &observer);
  void Detach(Observer &observer);
  void Notify(Topic topic);
};

