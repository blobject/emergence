#pragma once

#include <CL/cl2.hpp>
#include <iostream>
#include <math.h>
#include <string>

#include "../state/state.hh"


// DOGL: Macro for debugging OpenGL calls.

#define DOGL(x) Util::PrepDebugGl(); x; \
  if (! Util::DebugGl(#x, __FILE__, __LINE__)) __builtin_trap()


// Util: Static utility functions.

class Util
{
 public:
  // debug.cc
  static void DebugCl(cl_int err, const std::string &name);
  static void PrepDebugGl();
  static bool DebugGl(const std::string &func, const std::string &file,
                      int line);

  // io.cc
  static void Out(const std::string &s);
  static void Warn(const std::string &s);
  static void Err(const std::string &s);
  static void ErrCl(const std::string &s);
  static void ErrGl(const std::string &s);
  static bool LoadState(State &state, const std::string &path);
  static bool SaveState(State &state, const std::string &path);

  // math.cc
  template<typename T> static T Distribute(Distribution d, T a, T b);
  template<> int   Distribute<int>(Distribution d, int a, int b);
  template<> float Distribute<float>(Distribution d, float a, float b);
  static float DegToRad(float d);
  static float RadToDeg(float r);
  static int   ModI(int n, int m);
  static float ModF(float n, float m);
  static int   Signum(int n);

  // string.cc
  static std::string Relative(const std::string &path);
  static std::string Trim(std::string s);
};


/**
// Observer: The pattern

class Subject;

class Observer
{
 public:
  virtual ~Observer() = default;
  virtual void React(Subject &next) = 0;
};


// Subject: Part of the observer pattern

class Subject
{
 private:
  std::vector<Observer*> observers;

 public:
  virtual ~Subject() = default;
  void Attach(Observer &observer);
  void Detach(Observer &observer);
  void Notify();
  virtual unsigned int get(std::string key) = 0;
};
//*/

