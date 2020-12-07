#pragma once

#include <algorithm>
#include <vector>


// Observer design pattern

class Subject;

class Observer
{
 public:
  virtual ~Observer() = default;
  virtual void React(Subject&) = 0;
};

class Subject
{
 private:

 public:
  std::vector<Observer*> observers_;
  virtual ~Subject() = default;
  void Attach(Observer &observer);
  void Detach(Observer &observer);
  void Notify();
};

