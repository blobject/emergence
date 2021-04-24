#include "cl.hh"

#if 1 == CL_ENABLED

#include "../util/common.hh"


Cl::Cl(Log& log)
  : log_(log)
{
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;

  cl::Platform::get(&platforms);
  if (0 == platforms.size()) {
    log.add(Attn::Ecl, "No platform found.", true);
    return;
  }
  this->platform_ = platforms.front();
  for (auto& platform : platforms) {
    if ("FULL_PROFILE" != platform.getInfo<CL_PLATFORM_PROFILE>()) {
      continue;
    }
    this->platform_ = platform;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (0 == devices.size()) {
      continue;
    }
    this->device_ = devices.front();
    break;
  }
  std::string name = this->device_.getInfo<CL_DEVICE_NAME>();
  if (name.empty()) {
    log.add(Attn::Ecl, "No device found.", true);
    return;
  }

  this->context_ = cl::Context(this->device_);
  this->queue_ = cl::CommandQueue(this->context_, this->device_);
  this->max_cu_ = this->device_.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  this->max_freq_ = this->device_.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
  this->max_gmem_ = this->device_.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

  this->prep_seek();
  this->prep_move();

  log.add(Attn::O,
          "Started OpenCL module and found\n  device: " + name
          + "\n  max compute units:\t" + std::to_string(this->max_cu_)
          + "\n  max clock frequency:\t" + std::to_string(this->max_freq_)
          + " MHz\n  max global memory:\t"
          + std::to_string(this->max_gmem_ / 1024 / 1024) + " MB",
          true);
}



void
Cl::prep_seek()
{
  std::string code =
    "void inc(\n"
    "  volatile __global int* n\n"
    ") {\n"
    "  atomic_inc(n);\n"
    "}\n"
    "\n"
    "__kernel void particles_seek(\n"
    "  __private float W,\n"
    "  __private float H,\n"
    "  __private float SCOPE,\n"
    "  __private float ASCOPE,\n"
    "  __private int COLS,\n"
    "  __private int ROWS,\n"
    "  __private unsigned int GSTRIDE,\n"
    "  __global const uint* G,\n"
    "  __global const int* COL,\n"
    "  __global const int* ROW,\n"
    "  __global const float* PX,\n"
    "  __global const float* PY,\n"
    "  __global const float* PC,\n"
    "  __global const float* PS,\n"
    "  __global unsigned int* PN,\n"
    "  __global unsigned int* PAN,\n"
    "  __global unsigned int* PL,\n"
    "  __global unsigned int* PR\n"
    ") {\n"
    "  int srci = get_global_id(0);\n"
    "  int cc  = COL[srci];\n"
    "  int rr  = ROW[srci];\n"
    "  int c   = cc - 1;\n"
    "  int ccc = cc + 1;\n"
    "  int r   = rr - 1;\n"
    "  int rrr = rr + 1;\n"
    "  bool c_u = false;\n"
    "  bool c_o = false;\n"
    "  bool r_u = false;\n"
    "  bool r_o = false;\n"
    "  if      (cc == 0)        { c_u = true; c = COLS - 1; }\n"
    "  else if (cc == COLS - 1) { c_o = true; ccc = 0; }\n"
    "  if      (rr == 0)        { r_u = true; r = ROWS - 1; }\n"
    "  else if (rr == ROWS - 1) { r_o = true; rrr = 0; }\n"
    "  int vic[54] = {c,   r,   c_u,   false, r_u,   false,\n"
    "                 cc,  r,   false, false, r_u,   false,\n"
    "                 ccc, r,   false, c_o,   r_u,   false,\n"
    "                 c,   rr,  c_u,   false, false, false,\n"
    "                 cc,  rr,  false, false, false, false,\n"
    "                 ccc, rr,  false, c_o,   false, false,\n"
    "                 c,   rrr, c_u,   false, false, r_o,\n"
    "                 cc,  rrr, false, false, false, r_o,\n"
    "                 ccc, rrr, false, c_o,   false, r_o};\n"
    "  unsigned int stride;\n"
    "  int dsti;\n"
    "  float srcx;\n"
    "  float srcy;\n"
    "  float dstx;\n"
    "  float dsty;\n"
    "  float dx;\n"
    "  float dy;\n"
    "  float dist;\n"
    "  float srcc;\n"
    "  float srcs;\n"
    "  float dstc;\n"
    "  float dsts;\n"
    "  for (int v = 0; v < 54; v += 6) {\n"
    "    stride = (COLS * (vic[v + 1] * GSTRIDE)) + (vic[v] * GSTRIDE);\n"
    "    c_u = vic[v + 2];\n"
    "    c_o = vic[v + 3];\n"
    "    r_u = vic[v + 4];\n"
    "    r_o = vic[v + 5];\n"
    "    for (int p = 0; p < GSTRIDE; ++p) {\n"
    "      dsti = G[stride + p];\n"
    "      if (dsti == -1) {\n"
    "        break;\n"
    "      }\n"
    "      if (srci <= dsti) {\n"
    "        continue;\n"
    "      }\n"
    "      srcx = PX[srci];\n"
    "      dstx = PX[dsti];\n"
    "      dx = dstx - srcx;\n"
    "      if (c_u) { dx -= W; } else if (c_o) { dx += W; }\n"
    "      srcy = PY[srci];\n"
    "      dsty = PY[dsti];\n"
    "      dy = dsty - srcy;\n"
    "      if (r_u) { dy -= H; } else if (r_o) { dy += H; }\n"
    "      dist = (dx * dx) + (dy * dy);\n"
    "      if (SCOPE < dist) {\n"
    "        continue;\n"
    "      }\n"
    "      if (ASCOPE >= dist) {\n"
    "        inc(&PAN[srci]);\n"
    "        inc(&PAN[dsti]);\n"
    "      }\n"
    "      inc(&PN[srci]);\n"
    "      inc(&PN[dsti]);\n"
    "      srcc = PC[srci];\n"
    "      srcs = PS[srci];\n"
    "      dstc = PC[dsti];\n"
    "      dsts = PS[dsti];\n"
    "      if (0.0f > (dx * srcs) - (dy * srcc)) { inc(&PR[srci]); }\n"
    "      else                                  { inc(&PL[srci]); }\n"
    "      if (0.0f < (dx * dsts) - (dy * dstc)) { inc(&PR[dsti]); }\n"
    "      else                                  { inc(&PL[dsti]); }\n"
    /**
    // TODO: duplication and misses occur when modifying PLS/PRD/etc. in
    //       parallel, so passing in alternative neighborhood scope instead.
    "      dststride = NSTRIDE * dsti;\n"
    "      srcl = PL[srci];\n"
    "      srcr = PR[srci];\n"
    "      dstl = PL[dsti];\n"
    "      dstr = PR[dsti];\n"
    "      srcli = srcstride + srcl;\n"
    "      srcri = srcstride + srcr;\n"
    "      dstli = dststride + dstl;\n"
    "      dstri = dststride + dstr;\n"
    "      if (0.0f > (dx * srcs) - (dy * srcc)) {\n"
    "        if (NSTRIDE > srcr) {\n"
    "          PRS[srcri] = dsti;\n"
    "          PRD[srcri] = dist;\n"
    "        }\n"
    "        inc(&PR[srci]);\n"
    "      }\n"
    "      else {\n"
    "        if (NSTRIDE > srcl) {\n"
    "          PLS[srcli] = dsti;\n"
    "          PLD[srcli] = dist;\n"
    "        }\n"
    "        inc(&PL[srci]);\n"
    "      }\n"
    "      if (0.0f < (dx * dsts) - (dy * dstc)) {\n"
    "        if (NSTRIDE > dstr) {\n"
    "          PRS[dstri] = srci;\n"
    "          PRD[dstri] = dist;\n"
    "        }\n"
    "        inc(&PR[dsti]);\n"
    "      }\n"
    "      else {\n"
    "        if (NSTRIDE > dstl) {\n"
    "          PLS[dstli] = srci;\n"
    "          PLD[dstli] = dist;\n"
    "        }\n"
    "        inc(&PL[dsti]);\n"
    "      }\n"
    //*/
    "    }\n"
    "  }\n"
    "}\n";

  Log& log = this->log_;
  try {
    std::string name = "particles_seek";
    cl::Program program(this->context_, code, CL_TRUE);
    int compile_err;
    this->kernel_seek_ = cl::Kernel(program, name.c_str(), &compile_err);
    if (compile_err) {
      log.add(Attn::Ecl, std::to_string(compile_err) + ": failed to compile '"
              + name + "'.", true);
    }
  } catch (cl_int err) {
    this->log_.add(Attn::Ecl, std::to_string(err), true);
  }
}


void
Cl::seek(unsigned int n, unsigned int w, unsigned int h,
         float scope, float ascope, int cols, int rows,
         unsigned int grid_stride, std::vector<int>& grid,
         std::vector<int>& gcol, std::vector<int>& grow,
         std::vector<float>& px, std::vector<float>& py,
         std::vector<float>& pc, std::vector<float>& ps,
         std::vector<unsigned int>& pn, std::vector<unsigned int>& pan,
         std::vector<unsigned int>& pl, std::vector<unsigned int>& pr)
{
  const cl_uint float_size = n * sizeof(float);
  const cl_uint int_size = n * sizeof(int);
  const cl_uint uint_size = n * sizeof(unsigned int);
  try {
    cl::Buffer G(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                 cols * rows * grid_stride * sizeof(int), grid.data());
    cl::Buffer COL(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                   int_size, gcol.data());
    cl::Buffer ROW(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                   int_size, grow.data());
    cl::Buffer PX(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  float_size, px.data());
    cl::Buffer PY(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  float_size, py.data());
    cl::Buffer PC(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  float_size, pc.data());
    cl::Buffer PS(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  float_size, ps.data());
    cl::Buffer PN(this->context_, CL_MEM_READ_WRITE, uint_size);
    cl::Buffer PAN(this->context_, CL_MEM_READ_WRITE, uint_size);
    cl::Buffer PL(this->context_, CL_MEM_READ_WRITE, uint_size);
    cl::Buffer PR(this->context_, CL_MEM_READ_WRITE, uint_size);
    this->kernel_seek_.setArg( 0, static_cast<cl_float>(w));
    this->kernel_seek_.setArg( 1, static_cast<cl_float>(h));
    this->kernel_seek_.setArg( 2, static_cast<cl_float>(scope));
    this->kernel_seek_.setArg( 3, static_cast<cl_float>(ascope));
    this->kernel_seek_.setArg( 4, static_cast<cl_int>(cols));
    this->kernel_seek_.setArg( 5, static_cast<cl_int>(rows));
    this->kernel_seek_.setArg( 6, static_cast<cl_int>(grid_stride));
    this->kernel_seek_.setArg( 7, G);
    this->kernel_seek_.setArg( 8, COL);
    this->kernel_seek_.setArg( 9, ROW);
    this->kernel_seek_.setArg(10, PX);
    this->kernel_seek_.setArg(11, PY);
    this->kernel_seek_.setArg(12, PC);
    this->kernel_seek_.setArg(13, PS);
    this->kernel_seek_.setArg(14, PN);
    this->kernel_seek_.setArg(15, PAN);
    this->kernel_seek_.setArg(16, PL);
    this->kernel_seek_.setArg(17, PR);
    this->queue_.enqueueWriteBuffer(PN, CL_TRUE, 0, uint_size, pn.data());
    this->queue_.enqueueWriteBuffer(PAN, CL_TRUE, 0, uint_size, pan.data());
    this->queue_.enqueueWriteBuffer(PL, CL_TRUE, 0, uint_size, pl.data());
    this->queue_.enqueueWriteBuffer(PR, CL_TRUE, 0, uint_size, pr.data());
    this->queue_.enqueueNDRangeKernel(this->kernel_seek_,
                                      cl::NullRange, n, cl::NullRange);
    this->queue_.enqueueReadBuffer(PN, CL_TRUE, 0, uint_size, pn.data());
    this->queue_.enqueueReadBuffer(PAN, CL_TRUE, 0, uint_size, pan.data());
    this->queue_.enqueueReadBuffer(PL, CL_TRUE, 0, uint_size, pl.data());
    this->queue_.enqueueReadBuffer(PR, CL_TRUE, 0, uint_size, pr.data());
    this->queue_.finish();
  } catch (cl_int err) {
    this->log_.add(Attn::Ecl, std::to_string(err));
  }
}


void
Cl::prep_move()
{
  std::string code =
    "__kernel void particles_move(\n"
    "  __private float TAU,\n"
    "  __private float W,\n"
    "  __private float H,\n"
    "  __private float A,\n"
    "  __private float B,\n"
    "  __private float S,\n"
    "  __private float E,\n"
    "  __global const unsigned int* PN,\n"
    "  __global const unsigned int* PL,\n"
    "  __global const unsigned int* PR,\n"
    "  __global float* PX,\n"
    "  __global float* PY,\n"
    "  __global float* PF,\n"
    "  __global float* PC,\n"
    "  __global float* PS\n"
    ") {\n"
    "  int i = get_global_id(0);\n"
    "  int signum = (0 < (int)(PR[i] - PL[i])) - ((int)(PR[i] - PL[i]) < 0);\n"
    "  float f = fmod(PF[i] + A + (B * PN[i] * (float)signum) + E, TAU);\n"
    "  if (f < 0) { f += TAU; }\n"
    "  PF[i] = f;\n"
    "  PC[i] = native_cos(f);\n"
    "  PS[i] = native_sin(f);\n"
    "  float x = fmod(PX[i] + (S * PC[i]), W);\n"
    "  if (x < 0.0f) { x += W; }\n"
    "  PX[i] = x;\n"
    "  float y = fmod(PY[i] + (S * PS[i]), H);\n"
    "  if (y < 0.0f) { y += H; }\n"
    "  PY[i] = y;\n"
    "}\n";

  Log& log = this->log_;
  try {
    std::string name = "particles_move";
    cl::Program program(this->context_, code, CL_TRUE);
    int compile_err;
    this->kernel_move_ = cl::Kernel(program, name.c_str(), &compile_err);
    if (compile_err) {
      log.add(Attn::Ecl, std::to_string(compile_err) + ": failed to compile '"
              + name + "'.", true);
    }
  } catch (cl_int err) {
    this->log_.add(Attn::Ecl, std::to_string(err), true);
  }
}


void
Cl::move(unsigned int n, unsigned int w, unsigned int h,
         float a, float b, float s, float e,
         std::vector<unsigned int>& pn,
         std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
         std::vector<float>& px, std::vector<float>& py,
         std::vector<float>& pf,
         std::vector<float>& pc, std::vector<float>& ps)
{
  const cl_uint float_size = n * sizeof(float);
  const cl_uint uint_size = n * sizeof(unsigned int);
  try {
    cl::Buffer PN(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uint_size, pn.data());
    cl::Buffer PL(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uint_size, pl.data());
    cl::Buffer PR(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                  uint_size, pr.data());
    cl::Buffer PX(this->context_, CL_MEM_READ_WRITE, float_size);
    cl::Buffer PY(this->context_, CL_MEM_READ_WRITE, float_size);
    cl::Buffer PF(this->context_, CL_MEM_READ_WRITE, float_size);
    cl::Buffer PC(this->context_, CL_MEM_READ_WRITE, float_size);
    cl::Buffer PS(this->context_, CL_MEM_READ_WRITE, float_size);
    this->kernel_move_.setArg( 0, TAU);
    this->kernel_move_.setArg( 1, static_cast<cl_float>(w));
    this->kernel_move_.setArg( 2, static_cast<cl_float>(h));
    this->kernel_move_.setArg( 3, static_cast<cl_float>(a));
    this->kernel_move_.setArg( 4, static_cast<cl_float>(b));
    this->kernel_move_.setArg( 5, static_cast<cl_float>(s));
    this->kernel_move_.setArg( 6, static_cast<cl_float>(e));
    this->kernel_move_.setArg( 7, PN);
    this->kernel_move_.setArg( 8, PL);
    this->kernel_move_.setArg( 9, PR);
    this->kernel_move_.setArg(10, PX);
    this->kernel_move_.setArg(11, PY);
    this->kernel_move_.setArg(12, PF);
    this->kernel_move_.setArg(13, PC);
    this->kernel_move_.setArg(14, PS);
    this->queue_.enqueueWriteBuffer(PX, CL_TRUE, 0, float_size, px.data());
    this->queue_.enqueueWriteBuffer(PY, CL_TRUE, 0, float_size, py.data());
    this->queue_.enqueueWriteBuffer(PF, CL_TRUE, 0, float_size, pf.data());
    this->queue_.enqueueWriteBuffer(PC, CL_TRUE, 0, float_size, pc.data());
    this->queue_.enqueueWriteBuffer(PS, CL_TRUE, 0, float_size, ps.data());
    this->queue_.enqueueNDRangeKernel(this->kernel_move_,
                                      cl::NullRange, n, cl::NullRange);
    this->queue_.enqueueReadBuffer(PX, CL_TRUE, 0, float_size, px.data());
    this->queue_.enqueueReadBuffer(PY, CL_TRUE, 0, float_size, py.data());
    this->queue_.enqueueReadBuffer(PF, CL_TRUE, 0, float_size, pf.data());
    this->queue_.enqueueReadBuffer(PC, CL_TRUE, 0, float_size, pc.data());
    this->queue_.enqueueReadBuffer(PS, CL_TRUE, 0, float_size, ps.data());
    this->queue_.finish();
  } catch (cl_int err) {
    this->log_.add(Attn::Ecl, std::to_string(err));
  }
}

#endif /* CL_ENABLED */

