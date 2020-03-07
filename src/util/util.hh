#pragma once

#include "../state/state.hh"


// DOGL: Macro for debugging OpenGL calls.

#define DOGL(x) Util::PrepDebugGl(); x; \
  if (! Util::DebugGl(#x, __FILE__, __LINE__)) __builtin_trap()


// LoadShaderOut: For file.cc/LoadShader() to return multiple values.

struct LoadShaderOut
{
  std::string Vertex;
  std::string Geometry;
  std::string Fragment;
};


// Util: Static utility functions.

class Util
{
 public:
  // debug.cc
  static void        PrepDebugGl();
  static bool        DebugGl(const std::string &func, const std::string &file,
                             int line);

  // file.cc
  static LoadShaderOut LoadShader(const std::string &path);
  static bool          LoadState(State* state, const std::string &path);
  static bool          SaveState(State* state, const std::string &path);

  // string.cc
  static std::string Relative(const std::string &path);
  static std::string Trim(std::string s);
};

