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
  // io.cc
  static void PrepDebugGl();
  static bool DebugGl(const std::string &func, const std::string &file,
                      int line);
  static bool LoadState(State &state, const std::string &path);
  static bool SaveState(State &state, const std::string &path);

  // math.cc
  template<typename T> static T Distribute(T a, T b);
  template<> int   Distribute<int>(int a, int b);
  template<> float Distribute<float>(float a, float b);
  static float DegToRad(float d);
  static float RadToDeg(float r);
  static float RoundF(float f);
  static int   Signum(int n);

  // string.cc
  static std::string Relative(const std::string &path);
  static std::string Trim(std::string s);
};

