#include <GL/glew.h>
#include <fstream>
#include <sstream>

#include "common.hh"
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
    std::cerr << "Error (gl): " << error << " at "
              << Util::Relative(path) << ':'
              << std::to_string(line) << "\n  " << func
              << std::endl;
    return false;
  }
  return true;
}


/*

State file format

- Delimited by horizontal space (ie. ' ') and vertical space (ie. '\n')
- First line contains non-particle-specific data
- Second line and onwards contain particle data
- Namely:

  STOP WIDTH HEIGHT ALPHA BETA SCOPE SPEED
  0 X_0 Y_0 PHI_0 RAD_0
  1 X_1 Y_1 PHI_1 RAD_1
  ...

*/

// LoadState: Parse a file containing the initial State.

bool
Util::LoadState(State &state, const std::string &path)
{
  std::ifstream stream(path);
  if (! stream)
  {
    return false;
  }
  std::string line;
  std::istringstream linestream;
  std::getline(stream, line);
  if (! line.empty())
  {
    unsigned int stop;
    unsigned int width;
    unsigned int height;
    float alpha;
    float beta;
    float scope;
    float speed;
    linestream = std::istringstream(line);
    // on read failure, the parameters are left unchanged
    if (linestream >> stop) state.stop_ = stop;
    if (linestream >> width) state.width_ = width;
    if (linestream >> height) state.height_ = height;
    if (linestream >> alpha) state.alpha_ = alpha;
    if (linestream >> beta) state.beta_ = beta;
    if (linestream >> scope) state.scope_ = scope;
    if (linestream >> speed) state.speed_ = speed;
  }
  unsigned int i;
  float px;
  float py;
  float pf;
  float prad;
  unsigned int count = 0;
  state.Clear();
  while (std::getline(stream, line))
  {
    if (line.empty())
    {
      continue;
    }
    linestream = std::istringstream(line);
    linestream >> i; // ignore the leading particle index
    if (! (linestream >> px))
    {
      px = Util::Distribute<float>(0.0f, static_cast<float>(state.width_));
    }
    state.px_.push_back(px);
    if (! (linestream >> py))
    {
      py = Util::Distribute<float>(0.0f, static_cast<float>(state.height_));
    }
    state.py_.push_back(py);
    if (! (linestream >> pf))
    {
      pf = Util::Distribute<float>(0.0f, TAU);
    }
    state.pf_.push_back(pf);
    if (! (linestream >> prad)) { prad = 2.0f; }
    state.prad_.push_back(prad);
    state.pc_.push_back(cosf(state.pf_.back()));
    state.ps_.push_back(sinf(state.pf_.back()));
    state.pn_.push_back(0);
    state.pl_.push_back(0);
    state.pr_.push_back(0);
    state.pgcol_.push_back(0);
    state.pgrow_.push_back(0);
    ++count;
  }
  state.num_ = count;
  if (count == 0)
  {
    state.num_ = 1000;
    state.Spawn();
  }
  return true;
}


// SaveState: Write the current State to a file.

bool
Util::SaveState(State &state, const std::string &path)
{
  std::ofstream stream(path);
  if (! stream)
  {
    return false;
  }
  stream << state.stop_ << ' '
         << state.width_ << ' '
         << state.height_ << ' '
         << state.alpha_ << ' '
         << state.beta_ << ' '
         << state.scope_ << ' '
         << state.speed_ << '\n';
  for (unsigned int i = 0; i < state.num_; ++i)
  {
    stream << i << ' '
           << state.px_[i] << ' '
           << state.py_[i] << ' '
           << state.pf_[i] << ' '
           << state.prad_[i] << '\n';
  }
  stream.close();
  return true;
}

