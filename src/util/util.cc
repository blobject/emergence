#include "common.hh"
#include "util.hh"


bool
Util::debug_gl(const std::string& func, const std::string& path, int line)
{
  std::string error;
  while (GLenum e = glGetError()) {
    if      (GL_INVALID_ENUM      == e) { error = "invalid enum"; }
    else if (GL_INVALID_VALUE     == e) { error = "invalid value"; }
    else if (GL_INVALID_OPERATION == e) { error = "invalid operation"; }
    else if (GL_STACK_OVERFLOW    == e) { error = "stack overflow"; }
    else if (GL_STACK_UNDERFLOW   == e) { error = "stack underflow"; }
    else if (GL_OUT_OF_MEMORY     == e) { error = "out of memory"; }
    else                                { error = "unknown"; }
    std::cerr << "Error(gl): " << error << " at "
              << std::regex_replace(path, std::regex("^.*[\\/]src[\\/]"), "")
              << ':' << std::to_string(line) << "\n  " << func << "\n";
    return false;
  }
  return true;
}


std::string
Util::trim(std::string s)
{
  s.erase(std::find_if_not(s.rbegin(), s.rend(),
                           [](int c) { return std::isspace(c); }).base(),
          s.end());
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(),
                                      [](int c) { return std::isspace(c); }));
  return s;
}

