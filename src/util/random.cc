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

