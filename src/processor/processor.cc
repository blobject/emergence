#include <iostream>
#include <CL/cl.hpp>
#include <glm/glm.hpp>

#include "processor.hh"


Processor::Processor(State* state)
  : state_(state)
{
  //InitCl();
}


void
Processor::Process()
{
}


// InitCl: Initialise OpenCL.

void
Processor::InitCl()
{
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;
  cl::Platform::get(&platforms);
  for (const auto &platform : platforms)
  {
    std::cout << "cl platform: " << platform.getInfo<CL_PLATFORM_NAME>()
              << "\n\tversion " << platform.getInfo<CL_PLATFORM_VERSION>()
              << std::endl;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    for (const auto &device : devices)
    {
      std::cout << "cl device: " << device.getInfo<CL_DEVICE_NAME>()
                << "\n\tversion " << device.getInfo<CL_DEVICE_VERSION>()
                << std::endl;
    }
  }
}

