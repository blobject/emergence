#include <iostream>
#include <fstream>
#include <sstream>

#include "util.hh"

ShaderSource
Util::LoadShader(std::string path)
{
  enum class Type { NONE = -1, VERTEX = 0, FRAGMENT = 1 };
  std::stringstream ss[2];
  std::ifstream stream(path);
  if (stream)
  {
    Type type = Type::NONE;
    std::string line;
    while(std::getline(stream, line))
    {
      if (line.find("#shader") != std::string::npos)
      {
        if (line.find("vertex") != std::string::npos)
        {
          type = Type::VERTEX;
        }
        else if (line.find("fragment") != std::string::npos)
        {
          type = Type::FRAGMENT;
        }
      }
      else
      {
        ss[(int) type] << line << '\n';
      }
    }
    stream.close();
  }
  else
  {
    return { "", "" };
  }
  return { ss[(int) Type::VERTEX].str(), ss[(int) Type::FRAGMENT].str()};
}

bool
Util::LoadState(State* state, std::string path)
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
      int distribution_num;
      Distribution distribution;
      float alpha;
      float beta;
      float gamma;
      linestream = std::istringstream(line);
      // on read failure, the State members are left unchanged
      if (linestream >> stop) state->stop_ = stop;
      if (linestream >> width) state->width_ = width;
      if (linestream >> height) state->height_ = height;
      if (linestream >> distribution_num)
        state->distribution_ = (Distribution) distribution_num;
      if (linestream >> alpha) state->alpha_ = alpha;
      if (linestream >> beta) state->beta_ = beta;
      if (linestream >> gamma) state->gamma_ = gamma;
    }
    unsigned int x;
    unsigned int y;
    float phi;
    float speed;
    unsigned size;
    float neighsize;
    state->particles_.clear();
    while (std::getline(stream, line))
    {
      if (line.empty())
      {
        continue;
      }
      linestream = std::istringstream(line);
      Particle particle(state->rng_, state->distribution_,
                        state->width_, state->height_);
      if (linestream >> x) particle.x_ = x;
      if (linestream >> y) particle.y_ = y;
      if (linestream >> phi) particle.phi_ = phi;
      if (linestream >> speed) particle.speed_ = speed;
      if (linestream >> size) particle.size_ = size;
      if (linestream >> neighsize) particle.neighsize_ = neighsize;
      state->particles_.push_back(particle);
    }
    if (state->particles_.empty())
    {
      state->particles_ = std::vector<Particle>
        (1000, Particle(state->rng_, state->distribution_,
                        state->width_, state->height_));
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool
Util::SaveState(State* state, std::string path)
{
  std::ofstream stream(path);
  if (stream)
  {
    stream << state->stop_ << ' '
           << state->width_ << ' '
           << state->height_ << ' '
           << (int) state->distribution_ << ' '
           << state->alpha_ << ' '
           << state->beta_ << ' '
           << state->gamma_ << '\n';
    for (const Particle &particle : state->particles_)
    {
      stream << particle.x_ << ' '
             << particle.y_ << ' '
             << particle.phi_ << ' '
             << particle.speed_ << ' '
             << particle.size_ << ' '
             << particle.neighsize_ << '\n';
    }
    stream.close();
  }
  else
  {
    return false;
  }
  return true;
}

