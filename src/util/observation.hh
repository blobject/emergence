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
  std::vector<Observer*> observers_;

 public:
  virtual ~Subject() = default;
  void Attach(Observer &observer);
  void Detach(Observer &observer);
  void Notify();
};

