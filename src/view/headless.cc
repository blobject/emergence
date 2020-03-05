#include <iostream>

#include "headless.hh"

Headless::Headless(Processor* proc)
{
  this->proc_ = proc;
}

void
Headless::Process()
{
  std::cout << "TODO: call processor" << std::endl;
}

