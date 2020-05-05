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
Util::Signum(int n)
{
  return (0 < n) - (n < 0);
}

