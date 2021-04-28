//===-- util/util.hh - utility functions -----------------------*- C++ -*-===//
///
/// \file
/// Declarations of static utility functions.
///
//===---------------------------------------------------------------------===//

#pragma once

#include <GL/glew.h>
#include <iostream>
#include <math.h> // cosf, sinf, floor, fmod
#include <random>
#include <regex>
#include <unistd.h> // readlink


// DOGL: A macro for debugging OpenGL calls.

#define DOGL(x) Util::prep_debug_gl(); x; \
  if (!Util::debug_gl(#x, __FILE__, __LINE__)) __builtin_trap()


class Util
{
 public:
  // env //////////////////////////////////////////////////////////////////////

  /// execution_dir(): Get the directory at which program was spawned.
  /// \returns  string of directory at which program was spawned
  static std::string execution_dir();

  /// working_dir(): Get the parent process' current working directory.
  /// \returns  string of current working directory
  static std::string working_dir();

  // io ///////////////////////////////////////////////////////////////////////

  /// prep_debug_gl(): Clear out all OpenGL errors.
  inline static void
  prep_debug_gl()
  {
    while (GL_NO_ERROR != glGetError());
  }

  /// debug_gl(): Print the first OpenGL error.
  /// \param func  name of erroneous function
  /// \param path  path to the file where the error occurred
  /// \param line  line in the file where the error occurred
  /// \returns  true if no error occurred
  static bool debug_gl(const std::string& func, const std::string& path,
                       int line);

  // math /////////////////////////////////////////////////////////////////////

  /// distr(): Pick a number from a uniformly distributed range.
  /// \param a  start of range
  /// \param b  end of range
  /// \returns  uniformly distributed random number
  template<typename T> static inline T
  distr(T /* a */, T /* b */) {}

  /// int version of distr().
  template<> inline int
  distr<int>(int a, int b)
  {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> distribution(a, b);
    return distribution(rng);
  }

  /// float version of distr().
  template<> inline float
  distr<float>(float a, float b)
  {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_real_distribution<float> distribution(a, b);
    return distribution(rng);
  }

  /// deg_to_rad(): Convert from degrees to radians.
  /// \param d  angle in degrees
  /// \returns  angle in radians
  static inline float
  deg_to_rad(float d)
  {
    return d * M_PI / 180.0f;
  }

  /// rad_to_deg(): Convert from radians to degrees.
  /// \param r  angle in radians
  /// \returns  angle in degrees
  static inline float
  rad_to_deg(float r)
  {
    return r * 180.0f / M_PI;
  }

  /// floats_same(): Whether two floats are the same up to 3 decimal precision.
  ///                Used for State-UiState parameter comparisons.
  /// \param a  a float
  /// \param b  a float
  /// \returns  true if a = b up to 3 decimal precision
  static inline bool
  floats_same(float a, float b)
  {
    return std::round(a * 1000) / 1000.0f == std::round(b * 1000) / 1000.0f;
  }

  /// signum(): The signum function, used by Proc::plain_move().
  /// \param n  int
  /// \returns  signum of int
  static inline int
  signum(int n)
  {
    return (0 < n) - (n < 0);
  }

  /// normal_noise(): Gaussian noise of heading, used by experiment 5.
  /// \param
  static inline float
  normal_noise(float stddev)
  {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::normal_distribution<float> distribution(0.0f, stddev);
    return distribution(rng);
  }

  // string ///////////////////////////////////////////////////////////////////

  /// trim(): Remove whitespace at the ends of a string.
  /// \param s  string
  /// \returns  string with whitespace at the ends removed.
  static std::string trim(std::string s);
};

