#pragma once

#include "../state/state.hh"

#define DOGL(x) Util::PrepDebugGl(); x; \
  if (! Util::DebugGl(#x, __FILE__, __LINE__)) __builtin_trap()

struct ShaderSource
{
  std::string Vertex;
  std::string Fragment;
};

class Util
{
 public:
  static ShaderSource LoadShader(std::string path);
  static bool LoadState(State* state, std::string path);
  static bool SaveState(State* state, std::string path);
  static void PrepDebugGl();
  static bool DebugGl(std::string func, std::string file, int line);
  static std::string Relative(std::string path);
};

