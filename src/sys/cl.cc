#include "cl.hh"
#include "../util/common.hh"
#include "../util/util.hh"


Cl::Cl()
{
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;

  cl::Platform::get(&platforms);
  if (0 == platforms.size())
  {
    Util::Err("no cl platforms found.");
    return;
  }
  this->platform_ = platforms.front();
  for (auto &platform : platforms)
  {
    if ("FULL_PROFILE" != platform.getInfo<CL_PLATFORM_PROFILE>())
    {
      continue;
    }
    this->platform_ = platform;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (0 == devices.size())
    {
      continue;
    }
    this->device_ = devices.front();
    break;
  }
  if (this->device_.getInfo<CL_DEVICE_NAME>().empty())
  {
    Util::Err("no cl devices found.");
    return;
  }

  this->context_ = cl::Context(this->device_);
  this->queue_ = cl::CommandQueue(this->context_, this->device_);
  this->max_cu_ = this->device_.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  this->max_freq_ = this->device_.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
  this->max_gmem_ = this->device_.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

  this->PrepSeek();
  this->PrepMove();
}


bool
Cl::Good()
{
  return ! this->device_.getInfo<CL_DEVICE_NAME>().empty();
}


void
Cl::PrepSeek()
{
}


void
Cl::Seek()
{
}


void
Cl::PrepMove()
{
  std::string code =
    "__kernel void move_particles(\n"
    "  uint width,\n"
    "  uint height,\n"
    "  float alpha,\n"
    "  float beta,\n"
    "  float speed,\n"
    "  __global float* px,\n"
    "  __global float* py,\n"
    "  __global float* pf,\n"
    "  __global float* ps,\n"
    "  __global float* pc,\n"
    "  __global const unsigned int* pn,\n"
    "  __global const unsigned int* pl,\n"
    "  __global const unsigned int* pr,\n"
    "  float tau)\n"
    "{\n"
    "  int i = get_global_id(0);\n"
    "  int signum = (0 < (int)(pr[i] - pl[i])) - ((int)(pr[i] - pl[i]) < 0);\n"
    "  pf[i] = (float)fmod(pf[i] + alpha + (beta * pn[i] * (float)signum), tau);\n"
    "  if (pf[i] < 0) { pf[i] += tau; }\n"
    "  ps[i] = native_sin(pf[i]);\n"
    "  pc[i] = native_cos(pf[i]);\n"
    "  px[i] = (float)fmod(px[i] + (speed * pc[i]), (float)width);\n"
    "  if (px[i] < 0.0f) { px[i] += (float)width; }\n"
    "  py[i] = (float)fmod(py[i] + (speed * ps[i]), (float)height);\n"
    "  if (py[i] < 0.0f) { py[i] += (float)height; }\n"
    "}\n";
  /**
  for (auto &p : ps)
  {
    p.phi = Util::ModF(p.phi + a
                       + (b * p.n
                          * Util::Signum(static_cast<int>(p.r - p.l))), TAU);
    p.s = sinf(p.phi);
    p.c = cosf(p.phi);
    p.x = Util::ModI(p.x + static_cast<int>(s * p.c), w);
    p.y = Util::ModI(p.y + static_cast<int>(s * p.s), h);
  }
  //*/
  int err;
  try
  {
    cl::Program program(this->context_, code, CL_TRUE);
    this->kernel_move_ = cl::Kernel(program, "move_particles", &err);
    std::cout << err << std::endl;
  }
  catch (cl_int err) {
    Util::ErrCl(std::to_string(err));
  }
}


void
Cl::Move(unsigned int n, std::vector<float> &px, std::vector<float> &py,
         std::vector<float> &pf, std::vector<float> &ps, std::vector<float> &pc,
         std::vector<unsigned int> &pn,
         std::vector<unsigned int> &pl, std::vector<unsigned int> &pr,
         unsigned int w, unsigned int h, float a, float b, float s)
{
  const cl_uint floatsize = n * sizeof(float);
  const cl_uint uintsize = n * sizeof(unsigned int);
  try
  {
    cl::Buffer PX(this->context_, CL_MEM_READ_WRITE, floatsize);
    cl::Buffer PY(this->context_, CL_MEM_READ_WRITE, floatsize);
    cl::Buffer PF(this->context_, CL_MEM_READ_WRITE, floatsize);
    cl::Buffer PS(this->context_, CL_MEM_READ_WRITE, floatsize);
    cl::Buffer PC(this->context_, CL_MEM_READ_WRITE, floatsize);
    cl::Buffer PN(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uintsize, pn.data());
    cl::Buffer PL(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uintsize, pl.data());
    cl::Buffer PR(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uintsize, pr.data());
    this->kernel_move_.setArg( 0, static_cast<cl_uint>(w));
    this->kernel_move_.setArg( 1, static_cast<cl_uint>(h));
    this->kernel_move_.setArg( 2, static_cast<cl_float>(a));
    this->kernel_move_.setArg( 3, static_cast<cl_float>(b));
    this->kernel_move_.setArg( 4, static_cast<cl_float>(s));
    this->kernel_move_.setArg( 5, PX);
    this->kernel_move_.setArg( 6, PY);
    this->kernel_move_.setArg( 7, PF);
    this->kernel_move_.setArg( 8, PS);
    this->kernel_move_.setArg( 9, PC);
    this->kernel_move_.setArg(10, PN);
    this->kernel_move_.setArg(11, PL);
    this->kernel_move_.setArg(12, PR);
    this->kernel_move_.setArg(13, static_cast<cl_float>(TAU));
    this->queue_.enqueueWriteBuffer(PX, CL_TRUE, 0, floatsize, px.data());
    this->queue_.enqueueWriteBuffer(PY, CL_TRUE, 0, floatsize, py.data());
    this->queue_.enqueueWriteBuffer(PF, CL_TRUE, 0, floatsize, pf.data());
    this->queue_.enqueueWriteBuffer(PS, CL_TRUE, 0, floatsize, ps.data());
    this->queue_.enqueueWriteBuffer(PC, CL_TRUE, 0, floatsize, pc.data());
    this->queue_.enqueueNDRangeKernel(this->kernel_move_,
                                      cl::NullRange, n, cl::NullRange);
    this->queue_.enqueueReadBuffer(PX, CL_FALSE, 0, floatsize, px.data());
    this->queue_.enqueueReadBuffer(PY, CL_FALSE, 0, floatsize, py.data());
    this->queue_.enqueueReadBuffer(PF, CL_FALSE, 0, floatsize, pf.data());
    this->queue_.enqueueReadBuffer(PS, CL_FALSE, 0, floatsize, ps.data());
    this->queue_.enqueueReadBuffer(PC, CL_FALSE, 0, floatsize, pc.data());
    this->queue_.finish();
  }
  catch (cl_int err) {
    Util::ErrCl(std::to_string(err));
  }
}


void
Cl::Foo()
{
  /**/
  std::string code =
    "__kernel void\n"
    "foo(\n"
    "  uint n,\n"
    "  __global const float* a,\n"
    "  __global const float* b,\n"
    "  __global float* c)\n"
    "{\n"
    "  int i = get_global_id(0);\n"
    "  c[i] = a[i] + b[i];\n"
    "}\n";
  const int N = 1 << 4;
  int err;
  try
  {
    cl::Context context(this->device_);
    cl::CommandQueue queue(context, this->device_);
    cl::Program program(context, code, CL_TRUE);
    cl::Kernel foo(program, "foo", &err);
    std::cout << err << std::endl;
    std::vector<float> a(N, 12);
    std::vector<float> b(N, 30);
    std::vector<float> c(N);
    cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                a.size() * sizeof(float), a.data());
    cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                b.size() * sizeof(float), b.data());
    cl::Buffer C(context, CL_MEM_READ_WRITE, c.size() * sizeof(float));
    foo.setArg(0, static_cast<cl_int>(N));
    foo.setArg(1, A);
    foo.setArg(2, B);
    foo.setArg(3, C);
    queue.enqueueNDRangeKernel(foo, cl::NullRange, N, cl::NullRange);
    queue.enqueueReadBuffer(C, CL_FALSE, 0, c.size() * sizeof(float), c.data());
    queue.finish();
    /**
    for (int i = 0; i < c.size(); i++)
    {
      std::cout << c[i] << ", ";
    }
    std::cout << std::endl;
    //*/
  }
  catch (cl_int err) {
    Util::ErrCl("err");
  }
  //*/
}

