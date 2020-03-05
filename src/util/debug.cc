#include <iostream>
#include <regex>
#include <GL/glew.h>

#include "util.hh"

void
Util::PrepDebugGl()
{
  while (GL_NO_ERROR != glGetError());
}

bool
Util::DebugGl(std::string func, std::string path, int line)
{
  std::string error;
  while (GLenum e = glGetError())
  {
    switch (e)
    {
    case GL_INVALID_ENUM:      error = "invalid enum";      break;
    case GL_INVALID_VALUE:     error = "invalid value";     break;
    case GL_INVALID_OPERATION: error = "invalid operation"; break;
    case GL_STACK_OVERFLOW:    error = "stack overflow";    break;
    case GL_STACK_UNDERFLOW:   error = "stack underflow";   break;
    case GL_OUT_OF_MEMORY:     error = "out of memory";     break;
    default:                   error = "unknown";           break;
    }
    std::cerr << "Error: gl, " << error << " in " << func << "\n  "
              << Util::Relative(path) << ':' << line << std::endl;
    return false;
  }
  return true;
}

std::string
Util::Relative(std::string path)
{
  std::regex re("^.*[\\/](src[\\/])");
  return std::regex_replace(path, re, "$1");
}

