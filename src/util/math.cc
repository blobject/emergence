#include <random>

#include "util.hh"


template<typename T> T
Util::Distribute(T a, T b)
{
  //static_assert(false);
}

template<> int
Util::Distribute<int>(int a, int b)
{
  static std::random_device rd;
  static std::mt19937 rng(rd());
  std::uniform_int_distribution<int> distribution(a, b);
  return distribution(rng);
}

template<> float
Util::Distribute<float>(float a, float b)
{
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


// Keep float at 3 decimal precision for State-GuiState comparisons
float
Util::RoundF(float f)
{
  return std::round(f * 1000) / 1000.0f;
}


int
Util::Signum(int n)
{
  return (0 < n) - (n < 0);
}

