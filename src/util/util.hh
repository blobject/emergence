//===-- util.hh - utility functions -----------------------------*- C++ -*-===//
///
/// \file
/// Declarions of static utility functions.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <iostream>
#include <math.h> // cosf, sinf, floor, fmod
#include <random>
#include <string>


// DOGL: A macro for debugging OpenGL calls.

#define DOGL(x) Util::prep_debug_gl(); x; \
    if (!Util::debug_gl(#x, __FILE__, __LINE__)) __builtin_trap()


class Util
{
  public:
    //// io

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

    //// math

    /// distribute(): Pick a number from a uniformly distributed range.
    /// \param a  start of range
    /// \param b  end of range
    /// \returns  uniformly distributed random number
    template<typename T> static inline T
    distribute(T a, T b) {}

    /// int version of distribute().
    template<> inline int
    distribute<int>(int a, int b)
    {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<int> distribution(a, b);
        return distribution(rng);
    }

    /// float version of distribute().
    template<> inline float
    distribute<float>(float a, float b)
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

    /// round_float(): Round to 3 decimal precision, used for State-GuiState
    ///                comparisons.
    /// \param f  float
    /// \returns  float rounded to 3 decimal precision
    static inline float
    round_float(float f)
    {
        return std::round(f * 1000) / 1000.0f;
    }

    /// signum(): The signum function, used by Proc::plain_move().
    /// \param n  int
    /// \returns  signum of int
    static inline int
    signum(int n)
    {
        return (0 < n) - (n < 0);
    }


    //// string

    /// relative(): Get the file path relative to the project directory.
    /// \param path  path to the file
    /// \returns  path to the file relative to the project directory
    static std::string relative(const std::string& path);

    /// trim(): Remove whitespace at the ends of a string.
    /// \param s  string
    /// \returns  string with whitespace at the ends removed.
    static std::string trim(std::string s);
};

