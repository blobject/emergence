#pragma once

#include <CL/cl2.hpp>
#include <GL/glew.h>
#include <iostream>
#include <math.h> // cosf, sinf, floor, fmod
#include <random>
#include <string>


// DOGL: A macro for debugging OpenGL calls.

#define DOGL(x) Util::prep_debug_gl(); x; \
    if (! Util::debug_gl(#x, __FILE__, __LINE__)) __builtin_trap()


// Util: Static utility functions.

class Util
{
  public:
    //// io.cc
    // prep_debug_gl: Clear out all OpenGL errors.
    inline static void
    prep_debug_gl()
    {
        while (GL_NO_ERROR != glGetError());
    }
    static bool debug_gl(const std::string& func, const std::string& file,
                         int line);

    //// math.cc
    // distribute: Pick a number from a range in a uniform distribution.
    template<typename T> static inline T distribute(T a, T b) {}
    template<> inline int
    distribute<int>(int a, int b)
    {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<int> distribution(a, b);
        return distribution(rng);
    }
    template<> inline float
    distribute<float>(float a, float b)
    {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> distribution(a, b);
        return distribution(rng);
    }

    // deg_to_rad: Convert from degrees to radians.
    static inline float
    deg_to_rad(float d)
    {
        return d * M_PI / 180.0f;
    }

    // rad_to_deg: Convert from radians to degrees.
    static inline float
    rad_to_deg(float r)
    {
        return r * 180.0f / M_PI;
    }

    // round_float: 3 decimal precision, for State-GuiState comparisons.
    static inline float
    round_float(float f)
    {
        return std::round(f * 1000) / 1000.0f;
    }

    // signum: The signum function used by Proc::plain_move().
    static inline int
    signum(int n)
    {
        return (0 < n) - (n < 0);
    }

    // string.cc
    static std::string relative(const std::string& path);
    static std::string trim(std::string s);
};

