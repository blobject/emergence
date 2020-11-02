#include "util.hh"


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
Util::ModI(int n, int m)
{
  n %= m;
  if (n < 0)
  {
    n += m;
  }
  return n;
}


float
Util::ModF(float n, float m)
{
  n = fmod(n, m);
  if (n < 0)
  {
    n += m;
  }
  return n;
}


int
Util::Signum(int n)
{
  return (0 < n) - (n < 0);
}

