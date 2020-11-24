#include <random>

#include "util.hh"


template<typename T> T
Util::Distribute(Distribution d, T a, T b)
{
  //static_assert(false);
}

template<> int
Util::Distribute<int>(Distribution d, int a, int b)
{
  // TODO: d unused
  static std::random_device rd;
  static std::mt19937 rng(rd());
  std::uniform_int_distribution<int> distribution(a, b);
  return distribution(rng);
}

template<> float
Util::Distribute<float>(Distribution d, float a, float b)
{
  // TODO: d unused
  static std::random_device rd;
  static std::mt19937 rng(rd());
  std::uniform_real_distribution<float> distribution(a, b);
  return distribution(rng);
}


float
Util::DegToRad(float d)
{
  return d * M_PI / 180.0f;
}


float
Util::RadToDeg(float r)
{
  return r * 180.0f / M_PI;
}


int
Util::Signum(int n)
{
  return (0 < n) - (n < 0);
}

