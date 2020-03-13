#include <GL/glew.h>

#include "util.hh"


// PrepDebugGl: Clear out all OpenGL errors.

void
Util::PrepDebugGl()
{
  while (GL_NO_ERROR != glGetError());
}


// DebugGl: Print the first OpenGL error.

bool
Util::DebugGl(const std::string &func, const std::string &path, int line)
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
    Util::ErrGl(error + " at " + Util::Relative(path) + ':'
                + std::to_string(line) + "\n  " + func);
    return false;
  }
  return true;
}

