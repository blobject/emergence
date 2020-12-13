#include "cl.hh"
#include "../util/common.hh"


Cl::Cl(Log& log)
    : log_(log)
{
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    std::string message;

    cl::Platform::get(&platforms);
    if (0 == platforms.size()) {
        message = "No platform found.";
        log.add(Attn::Ecl, message);
        std::cout << "Error(cl): " << message << std::endl;
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
        message = "No device found.";
        log.add(Attn::Ecl, message);
        std::cout << "Error(cl): " << message << "\n";
        return;
    }

    this->context_ = cl::Context(this->device_);
    this->queue_ = cl::CommandQueue(this->context_, this->device_);
    this->max_cu_ = this->device_.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
    this->max_freq_ = this->device_.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
    this->max_gmem_ = this->device_.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

    message = "Using OpenCL\n  device: " + name
        + "\n  max compute units:\t" + std::to_string(this->max_cu_)
        + "\n  max clock frequency:\t" + std::to_string(this->max_freq_)
        + " MHz\n  max global memory:\t"
        + std::to_string(this->max_gmem_ / 1024 / 1024) + " MB";
    log.add(Attn::O, message);
    std::cout << message << "\n";

    this->prep_seek();
    this->prep_move();
}



// prep_seek: Pre-build the kernel for performing particle seeking.

void
Cl::prep_seek()
{
    // see also: Proc::plain_seek(),plain_seek_vicinity(),plain_seek_tally()
    std::string code =
        "__kernel void particles_seek(\n"
        "  float W,\n"
        "  float H,\n"
        "  float SCOPE,\n"
        "  __global const uint* G,\n"
        "  __global const float* PX,\n"
        "  __global const float* PY,\n"
        "  __global const float* PC,\n"
        "  __global const float* PS,\n"
        "  __global unsigned int* PN,\n"
        "  __global unsigned int* PL,\n"
        "  __global unsigned int* PR,\n"
        "  __global const int* COL,\n"
        "  __global const int* ROW,\n"
        "  int COLS,\n"
        "  int ROWS,\n"
        "  uint STRIDE)\n"
        "{\n"
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
        "  int dsti;\n"
        "  for (int v = 0; v < 54; v += 6) {\n"
        "    for (int p = 0; p < STRIDE; ++p) {\n"
        "      dsti = G[COLS * (vic[v+1] * STRIDE) + (vic[v] * STRIDE) + p];\n"
        "      if (dsti == -1) {"
        "        break;\n"
        "      }\n"
        "      if (srci <= dsti) {"
        "        continue;\n"
        "      }\n"
        "      c_u = vic[v + 2];\n"
        "      c_o = vic[v + 3];\n"
        "      r_u = vic[v + 4];\n"
        "      r_o = vic[v + 5];\n"
        "      float srcx = PX[srci];\n"
        "      float dstx = PX[dsti];\n"
        "      float dx = dstx - srcx;\n"
        "      if (c_u) { dx -= W; } else if (c_o) { dx += W; }\n"
        "      float srcy = PY[srci];\n"
        "      float dsty = PY[dsti];\n"
        "      float dy = dsty - srcy;\n"
        "      if (r_u) { dy -= H; } else if (r_o) { dy += H; }\n"
        "      if ((dx * dx) + (dy * dy) > SCOPE) {"
        "        continue;\n"
        "      }\n"
        "      ++PN[srci];\n"
        "      ++PN[dsti];\n"
        "      float srcc = PC[srci];\n"
        "      float srcs = PS[srci];\n"
        "      float dstc = PC[dsti];\n"
        "      float dsts = PS[dsti];\n"
        "      if (0.0f > (dx * srcs) - (dy * srcc)) { ++PR[srci]; }\n"
        "      else                                  { ++PL[srci]; }\n"
        "      if (0.0f < (dx * dsts) - (dy * dstc)) { ++PR[dsti]; }\n"
        "      else                                  { ++PL[dsti]; }\n"
        "    }\n"
        "  }\n"
        "}\n";

    int compile;
    try {
        cl::Program program(this->context_, code, CL_TRUE);
        this->kernel_seek_ = cl::Kernel(program, "particles_seek", &compile);
        //std::cout << "seek kernel: " << compile << std::endl;
    } catch (cl_int err) {
        this->log_.add(Attn::Ecl, std::to_string(err));
        std::cout << "Error(cl): " << std::to_string(err) << "\n";
    }
}


// seek: Execute particle seeking.

void
Cl::seek(std::vector<int>& grid, unsigned int grid_stride, unsigned int n,
         std::vector<float>& px, std::vector<float>& py,
         std::vector<float>& pc, std::vector<float>& ps,
         std::vector<unsigned int>& pn,
         std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
         std::vector<int>& pgcol, std::vector<int>& pgrow,
         int cols, int rows, unsigned int w, unsigned int h, float scope2)
{
    const cl_uint floatsize = n * sizeof(float);
    const cl_uint intsize = n * sizeof(int);
    const cl_uint uintsize = n * sizeof(unsigned int);
    try {
        cl::Buffer G(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     cols * rows * grid_stride * sizeof(int), grid.data());
        cl::Buffer PX(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      floatsize, px.data());
        cl::Buffer PY(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      floatsize, py.data());
        cl::Buffer PC(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      floatsize, pc.data());
        cl::Buffer PS(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      floatsize, ps.data());
        cl::Buffer COL(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       intsize, pgcol.data());
        cl::Buffer ROW(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       intsize, pgrow.data());
        cl::Buffer PN(this->context_, CL_MEM_READ_WRITE, uintsize);
        cl::Buffer PL(this->context_, CL_MEM_READ_WRITE, uintsize);
        cl::Buffer PR(this->context_, CL_MEM_READ_WRITE, uintsize);
        this->kernel_seek_.setArg( 0, static_cast<cl_float>(w));
        this->kernel_seek_.setArg( 1, static_cast<cl_float>(h));
        this->kernel_seek_.setArg( 2, static_cast<cl_float>(scope2));
        this->kernel_seek_.setArg( 3, G);
        this->kernel_seek_.setArg( 4, PX);
        this->kernel_seek_.setArg( 5, PY);
        this->kernel_seek_.setArg( 6, PC);
        this->kernel_seek_.setArg( 7, PS);
        this->kernel_seek_.setArg( 8, PN);
        this->kernel_seek_.setArg( 9, PL);
        this->kernel_seek_.setArg(10, PR);
        this->kernel_seek_.setArg(11, COL);
        this->kernel_seek_.setArg(12, ROW);
        this->kernel_seek_.setArg(13, static_cast<cl_int>(cols));
        this->kernel_seek_.setArg(14, static_cast<cl_int>(rows));
        this->kernel_seek_.setArg(15, static_cast<cl_int>(grid_stride));
        this->queue_.enqueueWriteBuffer(PN, CL_TRUE, 0, uintsize, pn.data());
        this->queue_.enqueueWriteBuffer(PL, CL_TRUE, 0, uintsize, pl.data());
        this->queue_.enqueueWriteBuffer(PR, CL_TRUE, 0, uintsize, pr.data());
        this->queue_.enqueueNDRangeKernel(this->kernel_seek_,
                                          cl::NullRange, n, cl::NullRange);
        this->queue_.enqueueReadBuffer(PN, CL_TRUE, 0, uintsize, pn.data());
        this->queue_.enqueueReadBuffer(PL, CL_TRUE, 0, uintsize, pl.data());
        this->queue_.enqueueReadBuffer(PR, CL_TRUE, 0, uintsize, pr.data());
        this->queue_.finish();
    } catch (cl_int err) {
        this->log_.add(Attn::Ecl, std::to_string(err));
    }
}


// prep_move: Pre-build the kernel for performing particle moving.

void
Cl::prep_move()
{
    // see also: Proc::plain_move()
    std::string code =
        "__kernel void particles_move(\n"
        "  float W,\n"
        "  float H,\n"
        "  float A,\n"
        "  float B,\n"
        "  float S,\n"
        "  __global float* PX,\n"
        "  __global float* PY,\n"
        "  __global float* PF,\n"
        "  __global float* PC,\n"
        "  __global float* PS,\n"
        "  __global const unsigned int* PN,\n"
        "  __global const unsigned int* PL,\n"
        "  __global const unsigned int* PR,\n"
        "  float TAU)\n"
        "{\n"
        "  int i = get_global_id(0);\n"
        "  int signum = (0 < (int)(PR[i]-PL[i])) - ((int)(PR[i]-PL[i]) < 0);\n"
        "  float f = fmod(PF[i] + A + (B * PN[i] * (float)signum), TAU);\n"
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
    int compile;
    try {
        cl::Program program(this->context_, code, CL_TRUE);
        this->kernel_move_ = cl::Kernel(program, "particles_move", &compile);
        //std::cout << "move kernel: " << compile << std::endl;
    } catch (cl_int err) {
        this->log_.add(Attn::Ecl, std::to_string(err));
        std::cout << "Error(cl): " << std::to_string(err) << "\n";
    }
}


// move: Execute particle moving.

void
Cl::move(unsigned int n, std::vector<float>& px, std::vector<float>& py,
         std::vector<float>& pf, std::vector<float>& pc, std::vector<float>& ps,
         std::vector<unsigned int>& pn,
         std::vector<unsigned int>& pl, std::vector<unsigned int>& pr,
         unsigned int w, unsigned int h, float a, float b, float s)
{
    const cl_uint floatsize = n * sizeof(float);
    const cl_uint uintsize = n * sizeof(unsigned int);
    try {
        cl::Buffer PX(this->context_, CL_MEM_READ_WRITE, floatsize);
        cl::Buffer PY(this->context_, CL_MEM_READ_WRITE, floatsize);
        cl::Buffer PF(this->context_, CL_MEM_READ_WRITE, floatsize);
        cl::Buffer PC(this->context_, CL_MEM_READ_WRITE, floatsize);
        cl::Buffer PS(this->context_, CL_MEM_READ_WRITE, floatsize);
        cl::Buffer PN(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      uintsize, pn.data());
        cl::Buffer PL(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      uintsize, pl.data());
        cl::Buffer PR(this->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      uintsize, pr.data());
        this->kernel_move_.setArg( 0, static_cast<cl_float>(w));
        this->kernel_move_.setArg( 1, static_cast<cl_float>(h));
        this->kernel_move_.setArg( 2, static_cast<cl_float>(a));
        this->kernel_move_.setArg( 3, static_cast<cl_float>(b));
        this->kernel_move_.setArg( 4, static_cast<cl_float>(s));
        this->kernel_move_.setArg( 5, PX);
        this->kernel_move_.setArg( 6, PY);
        this->kernel_move_.setArg( 7, PF);
        this->kernel_move_.setArg( 8, PC);
        this->kernel_move_.setArg( 9, PS);
        this->kernel_move_.setArg(10, PN);
        this->kernel_move_.setArg(11, PL);
        this->kernel_move_.setArg(12, PR);
        this->kernel_move_.setArg(13, static_cast<cl_float>(TAU));
        this->queue_.enqueueWriteBuffer(PX, CL_TRUE, 0, floatsize, px.data());
        this->queue_.enqueueWriteBuffer(PY, CL_TRUE, 0, floatsize, py.data());
        this->queue_.enqueueWriteBuffer(PF, CL_TRUE, 0, floatsize, pf.data());
        this->queue_.enqueueWriteBuffer(PC, CL_TRUE, 0, floatsize, pc.data());
        this->queue_.enqueueWriteBuffer(PS, CL_TRUE, 0, floatsize, ps.data());
        this->queue_.enqueueNDRangeKernel(this->kernel_move_,
                                          cl::NullRange, n, cl::NullRange);
        this->queue_.enqueueReadBuffer(PX, CL_TRUE, 0, floatsize, px.data());
        this->queue_.enqueueReadBuffer(PY, CL_TRUE, 0, floatsize, py.data());
        this->queue_.enqueueReadBuffer(PF, CL_TRUE, 0, floatsize, pf.data());
        this->queue_.enqueueReadBuffer(PC, CL_TRUE, 0, floatsize, pc.data());
        this->queue_.enqueueReadBuffer(PS, CL_TRUE, 0, floatsize, ps.data());
        this->queue_.finish();
    } catch (cl_int err) {
        this->log_.add(Attn::Ecl, std::to_string(err));
    }
}

