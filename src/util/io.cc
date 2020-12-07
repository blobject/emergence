#include <GL/glew.h>
#include <fstream>
#include <sstream>

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


// LoadState: Parse a file containing the initial state.

bool
Util::LoadState(State &state, const std::string &path)
{
  std::ifstream stream(path);
  if (stream)
  {
    std::string line;
    std::istringstream linestream;
    std::getline(stream, line);
    if (! line.empty())
    {
      unsigned int stop;
      unsigned int width;
      unsigned int height;
      float scope;
      float speed;
      float alpha;
      float beta;
      linestream = std::istringstream(line);
      // on read failure, the State members are left unchanged
      if (linestream >> stop) state.stop_ = stop;
      if (linestream >> width) state.width_ = width;
      if (linestream >> height) state.height_ = height;
      if (linestream >> scope) state.scope_ = scope;
      if (linestream >> speed) state.speed_ = speed;
      if (linestream >> alpha) state.alpha_ = alpha;
      if (linestream >> beta) state.beta_ = beta;
    }
    unsigned int x;
    unsigned int y;
    float phi;
    unsigned rad;
    /**
    state.particles_.clear();
    while (std::getline(stream, line))
    {
      if (line.empty())
      {
        continue;
      }
      linestream = std::istringstream(line);
      Particle particle(state.distribution_, state.width_,
                        state.height_);
      if (linestream >> x) particle.x = x;
      if (linestream >> y) particle.y = y;
      if (linestream >> phi) particle.phi = phi;
      if (linestream >> rad) particle.rad = rad;
      state.particles_.push_back(particle);
    }
    if (state.particles_.empty())
    {
      state.particles_ = std::vector<Particle>
        (1000, Particle(state.distribution_, state.width_,
                        state.height_));
    }
    //*/
  }
  else
  {
    return false;
  }
  return true;
}


// SaveState: Write the current state to a file.

bool
Util::SaveState(State &state, const std::string &path)
{
  /**
  std::ofstream stream(path);
  if (stream)
  {
    stream << state.stop_ << ' '
           << state.width_ << ' '
           << state.height_ << ' '
           << static_cast<int>(state.distribution_) << ' '
           << state.scope_ << ' '
           << state.speed_ << ' '
           << state.alpha_ << ' '
           << state.beta_ << '\n';
    for (const Particle &particle : state.particles_)
    {
      stream << particle.x << ' '
             << particle.y << ' '
             << particle.phi << ' '
             << particle.rad << '\n';
    }
    stream.close();
  }
  else
  {
    return false;
  }
  //*/
  return true;
}

