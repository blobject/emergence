#include "exp.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <algorithm>
#include <iomanip>


Exp::Exp(Log& log, State& state, Proc& proc, bool no_cl)
  : log_(log), state_(state), proc_(proc), no_cl_(no_cl)
{
  this->magentas_ = 0;
  this->blues_ = 0;
  this->yellows_ = 0;
  this->browns_ = 0;
  this->greens_ = 0;

  this->exp_4_est_done_ = 0;
  this->exp_4_dbscan_done_ = 0;
  this->exp_5_est_done_ = 0;
  this->exp_5_dbscan_done_ = 0;

  for (int p = 0; p < state.num_; ++p) {
    this->type_history_.push_back({});
  }

  // sprite particle positions begin at 0,0
  this->sprites_ = {
    {Type::Nutrient, gen_sprite({
      4.359f,  0.738f, 344.321f,
      0.0f,    4.791f, 154.132f,
      3.257f,  0.063f, 265.379f,
      2.836f,  6.01f,   37.821f,
      3.135f, 11.049f, 250.919f,
      8.233f,  7.329f, 201.614f,
      8.176f, 11.246f,  77.288f,
      5.587f, 11.373f,  30.078f,
      9.855f,  2.987f, 284.68f,
      9.56f,   4.878f, 273.469f,
      8.217f,  0.0f,   283.714f,
      7.159f,  5.38f,   12.862f,
      3.502f,  8.48f,   81.85f
    }, 13 * 3)},
    {Type::PrematureSpore, gen_sprite({
      4.455f, 4.518f, 206.494f,
      4.546f, 4.645f, 258.446f,
      4.339f, 4.140f, 172.188f,
      4.393f, 6.307f, 161.167f,
      5.346f, 5.576f, 197.525f,
      5.135f, 5.419f, 297.001f,
      5.577f, 5.449f,   8.7f,
      5.161f, 4.683f, 318.013f,
      5.264f, 5.665f,  48.591f,
      5.306f, 6.364f, 130.817f,
      5.772f, 5.870f, 120.316f,
      4.747f, 5.558f, 287.242f,
      4.930f, 5.749f, 155.978f,
      5.043f, 5.490f, 276.631f
    }, 14 * 3)},
    {Type::MatureSpore, gen_sprite({
      4.780f, 4.711f, 241.281f,
      7.179f, 4.232f, 162.044f,
      5.401f, 4.686f, 338.362f,
      5.127f, 5.021f,  18.795f,
      4.92f,  4.796f, 235.574f,
      6.569f, 5.673f,   0.659f,
      5.148f, 4.331f, 296.431f,
      5.112f, 5.1f,    48.975f,
      4.106f, 5.097f, 245.829f,
      5.081f, 4.49f,  281.333f,
      2.359f, 4.623f,  95.421f,
      4.517f, 4.963f, 174.283f,
      4.63f,  5.059f, 189.564f,
      4.602f, 4.992f, 125.523f,
      5.147f, 4.641f,  58.026f,
      4.572f, 4.474f,  95.979f,
      4.66f,  5.065f, 106.081f,
      5.228f, 5.064f,  40.971f,
      4.584f, 4.816f, 210.378f,
      5.161f, 4.953f,  43.443f
    }, 20 * 3)},
    {Type::Ring, gen_sprite({
       5.718f, 9.034f, 100.464f,
       5.531f, 8.625f, 158.659f,
       8.604f, 4.218f, 307.679f,
       9.693f, 4.94f,  320.587f,
       5.818f, 4.362f, 350.117f,
       5.963f, 8.181f,   9.227f,
       9.074f, 8.340f, 113.41f,
       7.522f, 9.008f, 249.45f,
       5.03f,  8.242f, 129.577f,
       5.513f, 7.982f,  14.729f,
       9.979f, 5.23f,  240.502f,
       5.826f, 8.715f, 322.513f,
       6.148f, 9.325f, 150.277f,
       5.327f, 9.552f,  44.875f,
       8.879f, 4.839f, 330.891f,
      10.289f, 6.389f, 262.653f,
      10.511f, 2.992f,  71.739f,
       6.67f,  3.705f, 197.424f,
       9.624f, 4.482f, 247.012f,
       6.493f, 9.217f,  53.889f,
       4.497f, 6.470f,  81.982f,
       9.411f, 4.502f,  99.715f,
       7.396f, 3.807f,  79.095f,
       9.484f, 8.846f, 141.48f,
       7.476f, 4.118f, 163.734f,
       9.801f, 5.381f, 349.456f,
       9.409f, 4.692f, 252.148f,
       5.951f, 8.907f, 345.539f
    }, 28 * 3)},
    {Type::PrematureCell, gen_sprite({
       8.422f, 11.203f,  29.511f,
       7.488f, 11.257f,   7.71f,
       7.233f,  5.716f,  87.112f,
       8.382f, 12.115f, 314.466f,
       6.566f, 10.926f,  86.821f,
       4.674f,  8.687f, 343.834f,
       7.208f,  5.505f, 194.493f,
       9.291f,  6.279f,  82.318f,
       4.668f,  6.992f, 254.827f,
       9.29f,  10.53f,  301.463f,
       8.416f, 10.689f, 139.467f,
       3.908f,  8.548f, 122.547f,
       8.665f,  5.071f, 299.232f,
       8.521f,  5.044f, 279.886f,
       7.91f,  11.028f, 323.963f,
      11.735f, 10.203f, 119.482f,
       7.107f,  8.723f, 206.464f,
       8.685f, 11.137f,  26.133f,
      11.406f,  9.155f, 169.808f,
       5.138f,  8.193f,  22.101f,
      10.018f,  5.905f,  73.29f,
       8.740f, 10.513f, 242.368f,
       6.212f, 10.533f,  41.548f,
       9.367f,  6.054f,  68.705f,
       4.519f,  8.145f, 303.368f,
       7.818f, 11.667f, 325.635f,
       9.03f,  10.48f,  347.324f,
       8.119f,  5.586f, 131.24f,
      12.334f,  7.778f,  14.031f,
       8.119f,  7.054f, 213.896f,
       8.506f, 10.328f, 230.758f,
       8.713f,  5.834f, 144.364f,
       7.902f,  4.816f, 226.65f,
       9.352f,  6.224f,  62.921f,
       9.313f,  7.699f,  39.898f,
       4.145f,  7.107f, 233.579f,
       8.25f,   5.723f,  41.849f
    }, 37 * 3)},
    {Type::TriangleCell, gen_sprite({
       5.602f,  9.543f,   0.187f,
       4.968f,  7.791f, 228.55f,
      10.046f,  6.213f,  99.356f,
       5.756f, 10.857f, 273.178f,
      11.15f,  10.417f,  65.005f,
       5.263f,  5.837f, 188.492f,
       5.604f,  7.721f, 267.927f,
      10.589f,  6.334f, 344.47f,
       9.736f, 11.414f, 300.701f,
       5.461f,  8.439f, 354.935f,
       5.382f,  6.237f,  85.689f,
       8.724f,  5.379f, 138.772f,
       5.567f,  9.587f,  62.457f,
      10.504f,  7.232f, 145.526f,
       7.144f,  4.771f, 179.598f,
      11.214f,  8.915f, 215.737f,
       5.382f, 10.193f,  56.882f,
      10.524f,  6.888f, 100.884f,
      10.27f,  10.805f, 230.819f,
       9.597f, 11.873f, 358.337f,
      11.038f,  6.696f, 320.339f,
      11.779f,  8.261f,  92.849f,
       5.455f,  7.872f,  24.363f,
       5.572f,  8.586f,   3.77f,
       5.86f,   9.064f, 237.752f,
       6.241f, 13.114f,  28.941f,
      10.314f,  6.257f, 258.324f,
      10.154f,  5.964f,  94.866f,
       9.108f, 11.259f, 350.323f,
       8.236f,  8.952f, 108.303f,
      12.175f,  7.817f, 322.619f,
       7.853f,  4.386f, 239.988f,
       8.828f, 12.593f,  56.948f,
      10.458f, 10.331f, 187.717f,
      11.302f,  9.907f,  60.789f,
       8.263f, 11.659f, 215.626f,
       5.092f, 10.467f, 275.896f,
       8.852f,  5.344f, 233.867f,
       5.116f,  6.35f,  162.865f,
       5.553f, 11.086f, 155.227f
    }, 40 * 3)},
    {Type::SquareCell, gen_sprite({
       7.549f, 11.705f,  70.718f,
      10.653f,  7.448f, 225.429f,
      10.5f,    3.588f,  60.474f,
       6.041f,  4.357f, 272.794f,
       6.324f, 11.38f,  117.107f,
       5.867f,  7.783f, 123.225f,
      13.55f,   6.105f, 251.425f,
       9.925f, 10.017f,   2.765f,
      11.362f,  4.349f,  71.79f,
      11.646f,  9.863f,  56.154f,
       6.418f, 11.326f,  16.648f,
      10.636f, 12.128f,  47.968f,
       5.2f,    5.841f, 310.261f,
      11.092f,  3.06f,  269.926f,
       4.367f,  7.48f,  162.12f,
      13.255f,  7.749f, 295.076f,
      11.569f,  4.735f, 153.024f,
      11.425f,  3.778f, 289.297f,
       5.489f,  4.971f, 106.381f,
      13.657f,  6.911f, 235.954f,
       8.934f,  3.09f,  101.557f,
      10.902f,  9.3f,   192.614f,
      11.251f, 10.005f,  75.275f,
      12.04f,   5.213f, 184.188f,
       4.948f,  9.777f, 108.529f,
       5.878f,  3.992f, 297.855f,
       7.708f,  3.442f, 351.794f,
      11.431f,  4.957f, 155.618f,
       4.186f,  7.843f, 162.75f,
      10.677f, 10.254f,  43.245f,
      11.31f,   4.306f, 127.177f,
       9.537f,  6.438f,  57.839f,
       6.009f,  4.703f,  74.308f,
       9.097f, 11.379f,  84.583f,
       5.669f,  4.802f, 130.817f,
       6.182f,  7.446f, 210.814f,
      13.177f,  5.341f, 340.059f,
       5.911f,  4.094f, 344.028f,
       9.907f, 10.358f,  37.5f,
      10.965f,  3.877f, 200.981f,
       5.223f,  9.528f, 163.26f,
       7.269f,  2.646f, 250.674f,
       6.175f,  9.959f, 328.576f,
       6.64f,   2.759f, 221.908f,
      11.725f,  9.853f,  43.051f,
      11.186f,  9.808f, 172.481f,
       4.552f,  6.815f, 236.416f,
       5.572f,  9.653f, 156.415f,
       8.313f,  2.695f, 119.206f,
       5.294f,  7.862f, 125.097f
    }, 50 * 3)},
    {Type::PentagonCell, gen_sprite({
       8.539f, 12.659f, 106.297f,
      14.451f, 11.479f,  27.024f,
       5.889f,  8.069f, 310.496f,
       7.564f,  9.784f,  10.583f,
       6.015f,  9.808f, 197.002f,
      14.018f, 12.43f,   40.563f,
       7.869f,  3.853f,  26.837f,
       4.494f, 10.184f, 258.003f,
       9.813f, 12.062f, 257.274f,
      11.552f,  8.021f, 346.974f,
       9.409f, 12.697f, 190.785f,
       5.63f,   7.46f,  292.993f,
      10.569f,  4.884f, 320.811f,
       9.549f, 12.329f, 199.586f,
      11.129f,  4.322f, 350.245f,
      10.023f, 13.094f,  79.837f,
       6.191f,  8.478f,  34.017f,
       9.62f,   4.229f,  89.263f,
      13.865f,  9.21f,  154.488f,
      12.333f,  5.192f, 206.381f,
      14.238f,  9.439f, 309.043f,
      13.054f,  7.243f, 146.111f,
      13.504f,  7.873f, 280.266f,
       7.94f,   3.19f,  279.938f,
       6.621f,  4.06f,  194.58f,
      12.64f,   6.132f, 148.031f,
       9.236f,  4.61f,  341.163f,
       4.933f,  6.663f,  75.806f,
       6.705f,  3.731f, 175.014f,
       9.944f, 12.489f, 332.219f,
      12.232f,  4.38f,  359.104f,
      12.715f,  6.194f,  43.93f,
       5.995f,  4.524f, 313.17f,
       6.278f,  8.925f,  75.947f,
       9.202f, 12.669f, 179.974f,
       9.633f,  4.609f,  79.897f,
       7.156f,  6.697f, 223.502f,
      13.646f, 11.515f, 160.507f,
      10.288f,  4.452f,  35.657f,
      13.48f,   9.501f,  63.705f,
      12.877f,  8.029f, 188.594f,
       4.867f, 12.588f, 144.877f,
       5.715f,  8.072f, 359.814f,
       5.959f,  8.888f,  91.668f,
       5.409f,  6.922f, 348.012f,
      11.783f,  4.381f, 358.834f,
      10.394f, 12.487f,  36.178f,
      14.834f,  5.854f,  56.4f,
      13.44f,   9.519f, 194.675f,
       9.234f, 12.916f, 139.715f,
      12.983f,  7.874f, 221.491f,
       5.802f,  8.245f, 223.653f,
       9.072f, 12.04f,  227.62f,
       9.62f,  11.054f, 235.727f,
       8.503f,  3.395f, 299.429f,
      11.8f,    8.431f,  49.539f,
       8.199f, 10.209f, 132.405f
    }, 57 * 3)}
  };

  this->greater_sprites_ = {
    {Type::Nutrient, gen_greater_sprite(Type::Nutrient, {}, 0)},
    {Type::PrematureSpore, gen_greater_sprite(Type::PrematureSpore, {
       0.0f,   8.056f, 249.787f,
       1.33f,  8.973f, 350.101f,
       8.561f, 0.0f,   351.31f,
      11.237f, 5.127f,  78.884f,
       1.158f, 1.267f, 141.76f,
      11.021f, 6.019f, 297.02f
    }, 6 * 3)},
    {Type::MatureSpore, gen_greater_sprite(Type::MatureSpore, {
      10.940f,  4.324f, 191.244f,
       8.442f, 12.548f,  80.446f,
      10.425f,  7.832f, 233.923f,
       0.092f,  1.034f,  24.485f,
       5.560f,  8.703f, 340.9f,
       0.016f,  8.947f, 325.306f,
       4.902f, 13.011f,  28.105f,
       0.777f,  7.225f, 257.7f,
       0.0f,   11.302f, 235.934f,
       2.668f,  0.0f,   340.45f
    }, 10 * 3)},
    {Type::Ring, gen_greater_sprite(Type::Ring, {
       0.0f,    6.598f,  99.997f,
       6.611f, 14.005f, 217.211f,
      12.2f,   11.194f, 253.067f,
      13.91f,   1.871f,  79.627f,
       2.978f, 13.762f, 273.062f,
      10.73f,  12.429f,  86.719f,
       4.008f,  0.0f,   105.302f
    }, 7 * 3)},
    {Type::PrematureCell, gen_greater_sprite(Type::PrematureCell, {
       7.704f,  0.0f,    55.129f,
      12.761f, 13.525f, 229.918f,
      16.746f,  5.762f,  40.122f,
       0.0f,    7.616f,  68.334f,
       2.432f,  4.394f, 127.891f,
       5.678f, 15.774f,  33.216f,
      16.9f,    7.824f, 294.727f
    }, 7 * 3)},
    {Type::TriangleCell, gen_greater_sprite(Type::TriangleCell, {
      16.842f,  9.0f,   302.572f,
       9.412f, 17.201f, 111.771f,
       0.0f,    7.903f, 315.154f,
       8.695f,  0.0f,   179.72f,
       1.021f,  3.223f, 171.595f,
       4.741f,  0.979f,  67.414f,
       1.048f, 11.272f, 273.497f,
       3.340f, 15.327f, 302.364f
    }, 8 * 3)},
    {Type::SquareCell, gen_greater_sprite(Type::SquareCell, {
      14.48f,   0.0f,   234.155f,
       0.0f,    5.57f,   37.312f,
       5.197f, 16.072f, 345.347f,
       9.123f,  0.163f, 129.309f,
       1.312f, 11.68f,    6.275f,
      14.476f, 12.024f, 179.857f,
       3.992f,  0.302f, 242.277f,
       3.602f, 15.003f,  65.112f,
      18.543f,  7.145f,  19.282f,
      11.356f, 15.959f, 331.237f
    }, 10 * 3)},
    {Type::PentagonCell, gen_greater_sprite(Type::PentagonCell, {
       0.603f, 11.836f, 354.065f,
       7.098f,  0.0f,     4.999f,
       3.345f,  1.370f, 340.184f,
      17.962f,  9.926f, 242.261f,
       8.812f, 17.825f, 293.366f,
      13.331f, 16.729f, 254.708f,
       0.0f,    5.912f, 132.838f,
      15.437f,  1.168f, 345.701f,
      17.415f,  1.821f, 134.487f,
       0.707f,  8.372f,  72.895f,
       0.676f, 13.008f,  64.827f
    }, 11 * 3)},
  };

  log.add(Attn::O, "Started experiment module.");
}


void
Exp::type()
{
  State& state = this->state_;
  bool cl = !this->no_cl_;
  std::vector<Type>& pt = state.pt_;
  std::vector<unsigned int>& pn = state.pn_;
  float ascope = state.ascope_squared_;
  this->magentas_ = 0;
  this->blues_ = 0;
  this->yellows_ = 0;
  this->browns_ = 0;
  this->greens_ = 0;
  unsigned int n;

  // cl
  std::vector<unsigned int>& pan = state.pan_;

  // non-cl
  std::vector<float>& pld = state.pld_;
  std::vector<float>& prd = state.prd_;
  unsigned int n_stride = state.n_stride_;

  for (unsigned int p = 0; p < state.num_; ++p) {
    n = pn[p];

#if 1 == CL_ENABLED

    if (cl) {
      if (15 <  n && 15 < pan[p]) {
        pt[p] = Type::MatureSpore;
        ++this->magentas_;
        continue;
      }
    } else {

#endif

      if (15 < n && 15 < plain_alt_neighborhood(pld, prd, p, n_stride, ascope))
      {
        pt[p] = Type::MatureSpore;
        ++this->magentas_;
        continue;
      }

#if 1 == CL_ENABLED

    }

#endif

    if (15 < n && n <= 35) {
      pt[p] = Type::CellHull;
      ++this->blues_;
      continue;
    }
    if (35 < n) {
      pt[p] = Type::CellCore;
      ++this->yellows_;
      continue;
    }
    if (13 <= n && n <= 15) {
      pt[p] = Type::PrematureSpore;
      ++this->browns_;
      continue;
    }
    pt[p] = Type::Nutrient;
    ++this->greens_;
  }
}


void
Exp::reset_exp()
{
  this->reset_cluster();
  this->reset_inject();
  this->reset_color(); // resetting color should go last
}


void
Exp::reset_color()
{
  this->color(Coloring::Original);
}


void
Exp::reset_cluster()
{
  this->proc_.neighbors_sets_.clear();
  this->proc_.neighbors_dists_.clear();
  this->nearest_neighbor_dists_.clear();
  this->cores_.clear();
  this->vague_.clear();
  this->clusters_.clear();
  this->palette_.clear();
  this->cell_clusters_.clear();
  this->spore_clusters_.clear();
  this->districts_.clear();
}


void
Exp::reset_inject()
{
  this->injected_.clear();
}


void
Exp::color(Coloring scheme)
{
  State& state = this->state_;
  unsigned int num = state.num_;
  std::vector<Type>& pt = state.pt_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<float>& xa = state.xa_;

  if (Coloring::Original == scheme) {
    for (unsigned int p = 0; p < num; ++p) {
      if (Type::MatureSpore == pt[p]) {
        // magenta
        xr[p] = 0.8f;
        xg[p] = 0.2f;
        xb[p] = 0.4f;
        xa[p] = 1.0f;
        continue;
      }
      if (Type::CellHull == pt[p]) {
        // blue
        xr[p] = 0.2f;
        xg[p] = 0.4f;
        xb[p] = 0.8f;
        xa[p] = 1.0f;
        continue;
      }
      if (Type::CellCore == pt[p]) {
        // yellow
        xr[p] = 0.8f;
        xg[p] = 0.8f;
        xb[p] = 0.0f;
        xa[p] = 1.0f;
        continue;
      }
      if (Type::PrematureSpore == pt[p]) {
        // brown
        xr[p] = 0.4f;
        xg[p] = 0.2f;
        xb[p] = 0.1f;
        xa[p] = 1.0f;
        continue;
      }
      // green (nutrient)
      xr[p] = 0.4f;
      xg[p] = 0.6f;
      xb[p] = 0.0f;
      xa[p] = 0.5f;
    }
    return;
  }

  float scope = state.scope_;

  if (Coloring::Dynamic == scheme) {
    for (int p = 0; p < num; ++p) {
      xr[p] = pn[p] / (scope / 1.5f);
      xg[p] = pn[p] / scope;
      xb[p] = 0.7f;
      xa[p] = 1.0f;
    }
    return;
  }

  if (Coloring::Cluster == scheme) {
    for (int p = 0; p < num; ++p) {
      xr[p] = 0.4f;
      xg[p] = 0.4f;
      xb[p] = 0.4f;
      xa[p] = 0.5f;
    }
    std::vector<float> color = {};
    this->palette_index_ = 0;
    for (std::set<int>& c : this->clusters_) {
      //if (this->is_cluster_type(Type::MatureSpore, c)) {
      //  for (int p : c) {
      //    xr[p] = 1.0f;
      //    xg[p] = 1.0f;
      //    xb[p] = 1.0f;
      //  }
      //  continue;
      //}
      ++this->palette_index_;
      color = this->palette_sample();
      for (int p : c) {
        xr[p] = color[0];
        xg[p] = color[1];
        xb[p] = color[2];
        xa[p] = 1.0f;
      }
    }
    return;
  }

  if (Coloring::Inspect == scheme) {
    for (int p = 0; p < num; ++p) {
      xr[p] = 0.4f;
      xg[p] = 0.4f;
      xb[p] = 0.4f;
      xa[p] = 0.5f;
    }
    for (int p : this->inspect_) {
      xr[p] = 1.0f;
      xg[p] = 1.0f;
      xb[p] = 1.0f;
      xa[p] = 1.0f;
    }
    return;
  }

  unsigned int threshold = 0;
  if      (Coloring::Density10 == scheme) { threshold = 10; }
  else if (Coloring::Density15 == scheme) { threshold = 15; }
  else if (Coloring::Density20 == scheme) { threshold = 20; }
  else if (Coloring::Density25 == scheme) { threshold = 25; }
  else if (Coloring::Density30 == scheme) { threshold = 30; }
  else if (Coloring::Density35 == scheme) { threshold = 35; }
  else if (Coloring::Density40 == scheme) { threshold = 40; }
  for (int p = 0; p < num; ++p) {
    if (threshold > pn[p]) {
      xr[p] = 0.4f;
      xg[p] = 0.4f;
      xb[p] = 0.4f;
      xa[p] = 0.5f;
    } else {
      xr[p] = 1.0f;
      xg[p] = 1.0f;
      xb[p] = 1.0f;
      xa[p] = 1.0f;
    }
  }
}


void
Exp::highlight(std::vector<unsigned int>& particles)
{
  this->inspect_ = particles;
}


void
Exp::cluster(float radius, unsigned int minpts)
{
  this->reset_cluster();
  this->dbscan_categorise(radius, minpts);
  this->dbscan_collect();
  this->type_clusters();
}


void
Exp::districts()
{
  std::unordered_map<int,std::vector<int>>& ns = this->proc_.neighbors_sets_;
  std::vector<std::set<int>>& districts = this->districts_;

  for (std::set<int>& cluster : this->clusters_) {
    std::set<int> district;
    for (int p : cluster) {
      for (int n : ns[p]) {
        district.insert(n);
      }
    }
    districts.push_back(district);
  }
}


void
Exp::inject(Type type, bool greater)
{
  this->reset_inject();

  State& state = this->state_;
  SpritePts& sprite = this->sprites_[type];
  if (greater) {
    sprite = this->greater_sprites_[type];
  }
  float w = static_cast<float>(state.width_);
  float h = static_cast<float>(state.height_);
  unsigned int n_stride = state.n_stride_;
  unsigned int size = sprite.size();
  float dist_x = Util::distr(0.0f, w);
  float dist_y = Util::distr(0.0f, h);
  float x;
  float y;
  unsigned int si = 0;
  unsigned int i_stride;

  for (int i = state.num_; i < state.num_ + size; ++i) {
    SpritePt& p = sprite[si];
    ++si;
    i_stride = n_stride * si;
    x = std::get<0>(p) + dist_x; if (w <= x) { x -= w; }
    y = std::get<1>(p) + dist_y; if (h <= y) { y -= h; }
    state.px_.push_back(x);
    state.py_.push_back(y);
    state.pf_.push_back(std::get<2>(p));
    state.pc_.push_back(std::get<3>(p));
    state.ps_.push_back(std::get<4>(p));
    state.pn_.push_back(0);
    state.pl_.push_back(0);
    state.pr_.push_back(0);
    state.pan_.push_back(0);
    for (int j = 0; j < n_stride; ++j) { state.pls_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { state.prs_.push_back(-1); }
    for (int j = 0; j < n_stride; ++j) { state.pld_.push_back(-1.0f); }
    for (int j = 0; j < n_stride; ++j) { state.prd_.push_back(-1.0f); }
    state.pt_.push_back(type);
    state.xr_.push_back(1.0f);
    state.xg_.push_back(1.0f);
    state.xb_.push_back(1.0f);
    state.xa_.push_back(1.0f);
    this->injected_.push_back(i);
  }
  state.num_ += size;
  this->sprite_x_ = dist_x;
  this->sprite_y_ = dist_y;
}


unsigned int
Exp::plain_alt_neighborhood(std::vector<float>& pld, std::vector<float>& prd,
                            unsigned int p, unsigned int n_stride,
                            float ascope)
{
  unsigned int pstride = n_stride * p;
  unsigned int count = 0;
  float dist;

  for (int i = 0; i < n_stride; ++i) {
    dist = pld[pstride + i];
    if (0 > dist) {
      break;
    }
    if (ascope >= dist) {
      ++count;
    }
  }
  for (int i = 0; i < n_stride; ++i) {
    dist = prd[pstride + i];
    if (0 > dist) {
      break;
    }
    if (ascope >= dist) {
      ++count;
    }
  }

  return count;
}


std::vector<float>
Exp::palette_sample()
{
  unsigned int index = this->palette_index_;
  std::vector<std::vector<float>>& colors = this->palette_;

  if (index > colors.size()) {
    colors.push_back({
      Util::distr(0.3f, 1.0f),
      Util::distr(0.3f, 1.0f),
      Util::distr(0.3f, 1.0f)
    });
  }

  return colors[index - 1];
}


void
Exp::nearest_neighbor_dists()
{
  State& state = this->state_;
  std::unordered_map<int,std::vector<float>>& nd =
    this->proc_.neighbors_dists_;
  float max = static_cast<float>(state_.width_);
  float nearest;

  for (int p = 0; p < state_.num_; ++p) {
    if (!nd[p].size()) {
      continue;
    }
    nearest = max;
    for (float dist : nd[p]) {
      if (nearest > dist) {
        nearest = dist;
      }
    }
    this->nearest_neighbor_dists_.push_back(nearest);
  }
}


void
Exp::record_types()
{
  State& state = this->state_;
  std::vector<Type>& pt = state.pt_;
  std::vector<std::vector<Type>>& history = this->type_history_;

  for (int p = 0; p < state.num_; ++p) {
    if (history[p].empty() || pt[p] != history[p].back()) {
      history[p].push_back(pt[p]);
    }
  }
}


void
Exp::dbscan_categorise(float radius, unsigned int minpts)
{
  std::unordered_map<int,std::vector<int>>& ns = this->proc_.neighbors_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<int>& vague = this->vague_;
  auto grid = std::vector<int>();
  int cols;
  int rows;
  unsigned int stride;

  this->proc_.plain_seek(radius, grid, cols, rows, stride,
                         &Proc::tally_neighbors);

  auto it = ns.begin();
  while (it != ns.end()) {
    if (minpts > it->second.size()) {
      vague.push_back(it->first);
    } else {
      cores.push_back(it->first);
    }
    ++it;
  }

/* pseudocode
def:
  N: set of sets of neighbors
input: PTS, RAD, MIN
output: N, CORES, VAGUE

categorise(PTS, RAD, MIN):
  N := {}
  CORES := {}
  VAGUE := {}
  foreach P in PTS:
    N_P := neighbors(P, PTS, RAD)
    if |N_P| == 0:
      continue
    add N_P to N
    add P to (|N_P| < MIN ? VAGUE : CORES)
  return N, CORES, VAGUE
*/
}


void
Exp::dbscan_collect()
{
  std::unordered_map<int,std::vector<int>>& ns = this->proc_.neighbors_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<std::set<int>>& clusters = this->clusters_;
  auto visited = std::unordered_set<int>();
  auto stack = std::unordered_set<int>();
  int q;

  for (int p : cores) {
    if (0 < visited.count(p)) {
      continue;
    }
    stack.insert(p);
    auto cluster = std::set<int>();
    while (!stack.empty()) {
      q = *stack.begin();
      stack.erase(stack.begin());
      if (0 < cluster.count(q)) {
        continue;
      }
      cluster.insert(q);
      for (int r : ns[q]) {
        if (std::find(cores.begin(), cores.end(), r) != cores.end() &&
            0 >= cluster.count(r)) {
          stack.insert(r);
        }
      }
    }
    for (int s : cluster) {
      visited.insert(s);
    }
    clusters.push_back(cluster);
  }

/* - no need to test if Q (popped from WORKING) is in NEWCLUSTER
 * invariant, nothing can enter working if it is in cluster
 * (change working to set)
 */

/* pseudocode
input: N, CORES
output: CLUSTERS

collect(N, CORES):
  CLUSTERS := {}
  VIS := {}
  foreach P in CORES:
    if P in VIS:
      continue
    NEWCLUSTER := {}
    WORKING := [P]
    while WORKING not empty:
      Q := pop from WORKING
      if Q in NEWCLUSTER:
        continue
      add Q to NEWCLUSTER
      foreach R in N[Q]:
        if R in CORES and R not in NEWCLUSTER:
          add R to WORKING
    foreach Q in NEWCLUSTER:
      add Q to VIS (ignore redundant)
    add NEWCLUSTER to CLUSTERS
  return CLUSTERS
--- ignore ---
    foreach Q in N[P]:
      if Q in CORES:
        add Q to C
        add Q to V
        continue
      if Q not in T:
        T[Q] := {}
      add C to T[Q]
    add C to S
  return S, T
*/
}


void
Exp::type_clusters() {
  std::vector<std::set<int>>& clusters = this->clusters_;
  int type;

  for (int i = 0; i < clusters.size(); ++i) {
    type = this->type_of_cluster(clusters[i]);
    if (0 > type) {
      this->spore_clusters_.insert(i);
    } else if (0 < type) {
      this->cell_clusters_.insert(i);
    }
  }
}


int
Exp::type_of_cluster(std::set<int>& cluster)
{
  unsigned int size = cluster.size();

  if (16 < size && size < 23) {
    return -1;
  }
  if (22 < size) {
    return 1;
  }

  return 0;
}


SpritePts
Exp::gen_sprite(std::vector<float> xyf, unsigned int num)
{
  float f;
  SpritePts ps;

  for (int i = 0; i < num; i += 3) {
    f = Util::deg_to_rad(xyf[i + 2]);
    ps.push_back({xyf[i], xyf[i + 1], f, cosf(f), sinf(f)});
  }

  return ps;
}


SpritePts
Exp::gen_greater_sprite(Type type, std::vector<float> xyf, unsigned int num)
{
  float f;
  SpritePts ps;

  for (auto p : this->sprites_[type]) {
    ps.push_back(p);
  }

  for (int i = 0; i < num; i += 3) {
    f = Util::deg_to_rad(xyf[i + 2]);
    ps.push_back({xyf[i], xyf[i + 1], f, cosf(f), sinf(f)});
  }

  return ps;
}


float
Exp::dhi()
{
  State& state = this->state_;
  unsigned int width = state.width_;
  unsigned int height = state.height_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  float scope = state.scope_;
  float scopesq = scope * scope;
  float dx;
  float dy;
  std::vector<std::vector<int>> pixels(width, std::vector<int>(height, 0));

  auto grid = std::vector<int>();
  int cols;
  int rows;
  unsigned int gstride;

  this->proc_.plot(scope, grid, cols, rows, gstride);

  unsigned int uw = width / cols;
  unsigned int uh = height / rows;
  unsigned int ux;
  unsigned int uy;
  int c;
  int cc;
  int r;
  int rr;
  bool cunder;
  bool cover;
  bool runder;
  bool rover;
  unsigned int stride;
  int p;

  for (int col = 0; col < cols; ++col) {
    c = col - 1;
    cc = col + 1;
    cunder = false;
    cover = false;
    if      (col == 0)        { cunder = true; c = cols - 1; }
    else if (col == cols - 1) { cover  = true; cc = 0; }
    for (int row = 0; row < rows; ++row) {
      r = row - 1;
      rr = row + 1;
      runder = false;
      rover = false;
      if      (row == 0)        { runder = true; r = rows - 1; }
      else if (row == rows - 1) { rover  = true; rr = 0; }
      int vic[54] = {/* sw */ c,   r,   cunder, false, runder, false,
                     /* s  */ col, r,   false,  false, runder, false,
                     /* se */ cc,  r,   false,  cover, runder, false,
                     /* w  */ c,   row, cunder, false, false,  false,
                     /* c  */ col, row, false,  false, false,  false,
                     /* e  */ cc,  row, false,  cover, false,  false,
                     /* nw */ c,   rr,  cunder, false, false,  rover,
                     /* n  */ col, rr,  false,  false, false,  rover,
                     /* ne */ cc,  rr,  false,  cover, false,  rover};
      for (int i = 0; i < uw; ++i) {
        for (int j = 0; j < uh; ++j) {
          ux = col * uw + j;
          uy = row * uh + i;
          for (unsigned int v = 0; v < 54; v += 6) {
            stride = (cols * (vic[v + 1] * gstride)) + (vic[v] * gstride);
            for (unsigned int gi = 0; gi < gstride; ++gi) {
              p = grid[stride + gi];
              if (0 > p) {
                break;
              }
              dx = px[p] - ux;
              if      (vic[v + 2]) { dx -= width; }
              else if (vic[v + 3]) { dx += width; }
              dy = py[p] - uy;
              if      (vic[v + 4]) { dy -= height; }
              else if (vic[v + 5]) { dy += height; }
              if (scopesq >= (dx * dx + dy * dy)) {
                ++pixels[uy][ux];
              }
            }
          }
        }
      }
    }
  }

  unsigned int dense = 0;
  for (std::vector<int> row : pixels) {
    for (int count : row) {
      if (14 < count) {
        ++dense;
      }
    }
  }

  return static_cast<float>(dense) / (state.width_ * state.height_);
}


void
Exp::do_meta_exp(unsigned int tick)
{
  if (tick % 50) {
    return;
  }
  float radius = 0.6f * this->state_.scope_;
  unsigned int minpts = 14;
  this->cluster(radius, minpts);
  std::cout << tick << ": "
            << this->magentas_              << " magenta(mature_spore), "
            << this->blues_                 << " blue(cell_hull), "
            << this->yellows_               << " yellow(cell_core), "
            << this->clusters_.size()       << " clusters, "
            << this->cell_clusters_.size()  << " cells, "
            << this->spore_clusters_.size() << " spores"
            << std::endl;
}


void
Exp::do_exp_1a(unsigned int tick) {
  if (0 == tick || 150 == tick) {
    float radius = 10.0f;
    unsigned int minpts = 14;

    this->cluster(radius, minpts);
    this->nearest_neighbor_dists_.clear();
    this->nearest_neighbor_dists();

    std::cout << tick << ":";
    for (float dist : this->nearest_neighbor_dists_) {
      std::cout << " " << dist;
    }
    std::cout << std::endl;
  }
}


void
Exp::do_exp_1b(unsigned int tick) {
  if (  0 == tick ||
       60 == tick ||
       90 == tick ||
      180 == tick ||
      400 == tick ||
      700 == tick)
  {
    float radius = 10.0f;
    unsigned int minpts = 14;

    this->cluster(radius, minpts);
    this->nearest_neighbor_dists_.clear();
    this->nearest_neighbor_dists();

    std::cout << tick << ":";
    for (float dist : this->nearest_neighbor_dists_) {
      std::cout << " " << dist;
    }
    std::cout << std::endl;
  }
}


void
Exp::do_exp_2(unsigned int tick) {
  if (tick % 100) {
    return;
  }
  State& state = this->state_;
  unsigned int num = state.num_;
  float radius = state.scope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);

  std::cout << std::fixed << std::setprecision(2)
            << tick << ": "
            << this->magentas_              << " mature_spores, "
            << this->blues_                 << " cell_hulls, "
            << this->yellows_               << " cell_cores "
            << "(dbscan " << radius << "," << minpts << ": "
            << this->clusters_.size()       << " clusters, "
            << this->cell_clusters_.size()  << " cells, "
            << this->spore_clusters_.size() << " spores, "
            << this->cores_.size()          << " cores, "
            << this->vague_.size()          << " vagues, "
            << num - cores_.size() - vague_.size() << " noise)"
            << std::endl;

  this->record_types();
  std::vector<std::vector<Type>>& history = this->type_history_;
  char t;
  if (100000 == tick || 1000000 == tick) {
    std::cout << "types: ";
    for (int p = 0; p < num; ++p) {
      std::cout << p;
      for (Type type : history[p]) {
        t = 'g';
        if      (Type::MatureSpore    == type) { t = 'm'; }
        else if (Type::CellHull       == type) { t = 'b'; }
        else if (Type::CellCore       == type) { t = 'y'; }
        else if (Type::PrematureSpore == type) { t = 'w'; }
        std::cout << " " << t;
      }
      if (p != num - 1) {
        std::cout << ",";
      }
    }
    std::cout << std::endl;
  }
}


void
Exp::do_exp_3(unsigned int tick) {
  State& state = this->state_;
  std::vector<Type>& pt = state.pt_;
  std::vector<unsigned int>& pl = state.pl_;
  std::vector<unsigned int>& pr = state.pr_;
  unsigned int num = this->injected_.size();
  unsigned int p;
  Type type;
  char t = 'x';

  std::cout << tick << ":";
  for (int i = 0; i < num; ++i) {
    // TODO: something's not right
    p = this->injected_[i];
    type = pt[p];
    if      (Type::Nutrient       == type) { t = 'g'; }
    else if (Type::PrematureSpore == type) { t = 'w'; }
    else if (Type::MatureSpore    == type) { t = 'm'; }
    else if (Type::CellHull       == type) { t = 'b'; }
    else if (Type::CellCore       == type) { t = 'y'; }
    std::cout << " " << p << " " << t << " " << pl[p] << " " << pr[p];
    if (num - 1 > i) {
      std::cout << ",";
    }
  }
  std::cout << std::endl;
}


bool
Exp::do_exp_4a(unsigned int tick) {
  // slight tolerance for Exp and injection to be applied
  if (10 > tick) {
    return false;
  }

  State& state = this->state_;
  unsigned int num = state.num_;
  unsigned int width = state.width_;
  unsigned int height = state.height_;
  unsigned int size = this->sprites_[Type::MatureSpore].size();
  float dpe = static_cast<float>(num - size) / width / height;
  float radius = state.ascope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);
  std::vector<std::set<int>>& clusters = this->clusters_;
  unsigned int num_clusters = clusters.size();

  if (25000 == tick) {
    if (!this->exp_4_est_done_) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "end";
    }
    if (!this->exp_4_dbscan_done_) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "end";
    }
    std::cout << std::fixed << std::setprecision(3) << " " << dpe << " "
              << std::fixed << std::setprecision(0)
              << this->exp_4_est_done_ << " "
              << this->exp_4_est_how_ << " est "
              << this->exp_4_dbscan_done_ << " "
              << this->exp_4_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  if (!this->exp_4_est_done_) {
    unsigned int cell_size = this->blues_ + this->yellows_;
    if (17 > this->magentas_ && 23 > cell_size) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "decayed";
    } else if (22 < this->magentas_ || 22 < cell_size) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "grew";
    }
  }

  if (!this->exp_4_dbscan_done_) {
    if (!num_clusters) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "decayed";
    } else if (this->cell_clusters_.size()) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "grew";
    }
  }

  if (this->exp_4_est_done_ && this->exp_4_dbscan_done_) {
    std::cout << std::fixed << std::setprecision(3) << " " << dpe << " "
              << std::fixed << std::setprecision(0)
              << this->exp_4_est_done_ << " "
              << this->exp_4_est_how_ << " est "
              << this->exp_4_dbscan_done_ << " "
              << this->exp_4_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  return false;
}


bool
Exp::do_exp_4b(unsigned int tick) {
  // slight tolerance for Exp and injection to be applied
  if (10 > tick) {
    return false;
  }

  State& state = this->state_;
  unsigned int num = state.num_;
  unsigned int width = state.width_;
  unsigned int height = state.height_;
  unsigned int size = this->sprites_[Type::TriangleCell].size();
  float dpe = static_cast<float>(num - size) / width / height;
  float radius = state.scope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);
  std::vector<std::set<int>>& clusters = this->clusters_;
  unsigned int num_clusters = clusters.size();

  if (25000 == tick) {
    if (!this->exp_4_est_done_) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "end";
    }
    if (!this->exp_4_dbscan_done_) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "end";
    }
    std::cout << std::fixed << std::setprecision(3) << " " << dpe << " "
              << std::fixed << std::setprecision(0)
              << this->exp_4_est_done_ << " "
              << this->exp_4_est_how_ << " est "
              << this->exp_4_dbscan_done_ << " "
              << this->exp_4_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  if (!this->exp_4_est_done_) {
    unsigned int cell_size = this->blues_ + this->yellows_;
    if (23 > cell_size) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "decayed";
    } else if (2.1f * 48 < cell_size) {
      this->exp_4_est_done_ = tick;
      this->exp_4_est_how_ = "replicated";
    }
  }

  if (!this->exp_4_dbscan_done_) {
    if (!num_clusters) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "died";
    } else if (!this->cell_clusters_.size()) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "decayed";
    } else if (1 < num_clusters) {
      this->exp_4_dbscan_done_ = tick;
      this->exp_4_dbscan_how_ = "replicated";
    }
  }

  if (this->exp_4_est_done_ && this->exp_4_dbscan_done_) {
    std::cout << std::fixed << std::setprecision(3) << " " << dpe << " "
              << std::fixed << std::setprecision(0)
              << this->exp_4_est_done_ << " "
              << this->exp_4_est_how_ << " est "
              << this->exp_4_dbscan_done_ << " "
              << this->exp_4_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  return false;
}


bool
Exp::do_exp_4c(unsigned int tick) {
  if (25000 != tick) {
    return false;
  }

  State& state = this->state_;
  unsigned int num = state.num_;
  unsigned int width = state.width_;
  unsigned int height = state.height_;
  unsigned int size = this->sprites_[Type::MatureSpore].size();
  if (43 == state.experiment_) {
    size = this->sprites_[Type::TriangleCell].size();
  }
  float dpe = static_cast<float>(num - size) / width / height;
  float radius = state.scope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);

  std::cout << this->exp_4_count_ << ": "
            << std::fixed << std::setprecision(3) << dpe << ": "
            << std::fixed << std::setprecision(0)
            << this->magentas_              << " mature_spores, "
            << this->blues_                 << " cell_hulls, "
            << this->yellows_               << " cell_cores "
            << "(dbscan " << radius << "," << minpts << ": "
            << this->clusters_.size()       << " clusters, "
            << this->cell_clusters_.size()  << " cells, "
            << this->spore_clusters_.size() << " spores, "
            << this->cores_.size()          << " cores, "
            << this->vague_.size()          << " vagues, "
            << num - cores_.size() - vague_.size() << " noise)"
            << std::endl;

  return true;
}


bool
Exp::do_exp_5a(unsigned int tick) {
  // slight tolerance for Exp and injection to be applied
  if (10 > tick) {
    return false;
  }

  State& state = this->state_;
  int e = state.experiment_;
  float radius = state.scope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);
  std::vector<std::set<int>>& clusters = this->clusters_;
  unsigned int num_clusters = clusters.size();
  std::unordered_map<int,int>& est_size_counts = this->exp_5_est_size_counts_;
  std::unordered_map<int,int>& dbscan_size_counts =
    this->exp_5_dbscan_size_counts_;
  int size;

  for (int c : this->cell_clusters_) {
    size = this->clusters_[c].size();
    if (dbscan_size_counts.find(size) == dbscan_size_counts.end()) {
      dbscan_size_counts[size] = 0;
    }
    ++dbscan_size_counts[size];
  }

  size = this->blues_ + this->yellows_;
  if (est_size_counts.find(size) == est_size_counts.end()) {
    est_size_counts[size] = 0;
  }
  ++est_size_counts[size];

  if (25000 == tick) {
    std::cout << this->exp_5_count_ << ": " << tick << " end est";
    int i = 0;
    for (std::pair<int,int> est_size_count : est_size_counts) {
      std::cout << (0 < i ? "," : "") << " " << est_size_count.first
                << " " << est_size_count.second;
      ++i;
    }
    std::cout << "; " << tick << " end dbscan";
    i = 0;
    for (std::pair<int,int> dbscan_size_count : dbscan_size_counts) {
      std::cout << (0 < i ? "," : "") << " " << dbscan_size_count.first
                << " " << dbscan_size_count.second;
      ++i;
    }
    std::cout << std::endl;
    est_size_counts.clear();
    dbscan_size_counts.clear();
    return true;
  }

  if (!this->exp_5_est_done_) {
    if (23 > size) {
      this->exp_5_est_done_ = tick;
      this->exp_5_est_how_ = "died";
    } else if (2.1f * 48 < size) {
      // should not grow
      this->exp_5_est_done_ = tick;
      this->exp_5_est_how_ = "grew!";
    }
  }

  if (!this->exp_5_dbscan_done_) {
    if (!num_clusters) {
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "died";
    } else if (!this->cell_clusters_.size()) {
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "decayed";
    } else if (1 < num_clusters) {
      // should not grow
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "grew!";
    }
  }

  if (this->exp_5_est_done_ && this->exp_5_dbscan_done_) {
    std::cout << this->exp_5_count_ << ": "
              << this->exp_5_est_done_ << " "
              << this->exp_5_est_how_ << " est";
    int i = 0;
    for (std::pair<int,int> est_size_count : est_size_counts) {
      std::cout << (0 < i ? "," : "") << " " << est_size_count.first
                << " " << est_size_count.second;
      ++i;
    }
    std::cout << "; " << this->exp_5_dbscan_done_ << " "
              << this->exp_5_dbscan_how_ << " dbscan";
    i = 0;
    for (std::pair<int,int> dbscan_size_count : dbscan_size_counts) {
      std::cout << (0 < i ? "," : "") << " " << dbscan_size_count.first
                << " " << dbscan_size_count.second;
      ++i;
    }
    std::cout << std::endl;
    est_size_counts.clear();
    dbscan_size_counts.clear();
    return true;
  }

  return false;
}


bool
Exp::do_exp_5b(unsigned int tick) {
  // slight tolerance for Exp and injection to be applied
  if (10 > tick) {
    return false;
  }

  State& state = this->state_;
  int e = state.experiment_;
  float radius = state.scope_;
  unsigned int minpts = 14;

  this->cluster(radius, minpts);
  std::vector<std::set<int>>& clusters = this->clusters_;
  unsigned int num_clusters = clusters.size();
  unsigned int noise = Util::rad_to_deg(state.noise_);
  int size = this->blues_ + this->yellows_;

  if (25000 == tick) {
    if (!this->exp_5_est_done_) {
      this->exp_5_est_done_ = tick;
      this->exp_5_est_how_ = "end";
    }
    if (!this->exp_5_dbscan_done_) {
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "end";
    }
    std::cout << std::fixed << std::setprecision(0) << " " << noise << " "
              << this->exp_5_est_done_ << " "
              << this->exp_5_est_how_ << " est "
              << this->exp_5_dbscan_done_ << " "
              << this->exp_5_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  if (!this->exp_5_est_done_) {
    if (23 > size) {
      this->exp_5_est_done_ = tick;
      this->exp_5_est_how_ = "died";
    } else if (2.1f * 48 < size) {
      // should not grow
      this->exp_5_est_done_ = tick;
      this->exp_5_est_how_ = "grew!";
    }
  }

  if (!this->exp_5_dbscan_done_) {
    if (!num_clusters) {
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "died";
    } else if (!this->cell_clusters_.size()) {
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "decayed";
    } else if (1 < num_clusters) {
      // should not grow
      this->exp_5_dbscan_done_ = tick;
      this->exp_5_dbscan_how_ = "grew";
    }
  }

  if (this->exp_5_est_done_ && this->exp_5_dbscan_done_) {
    std::cout << std::fixed << std::setprecision(0) << " " << noise << " "
              << this->exp_5_est_done_ << " "
              << this->exp_5_est_how_ << " est "
              << this->exp_5_dbscan_done_ << " "
              << this->exp_5_dbscan_how_ << " dbscan"
              << std::flush;
    return true;
  }

  return false;
}


bool
Exp::do_exp_6(unsigned int tick) {
  if (500 != tick) {
    return false;
  }

  State& state = this->state_;

  std::cout << std::fixed << std::setprecision(0)
            << "alpha=" << Util::rad_to_deg(state.alpha_)
            << ",beta=" << Util::rad_to_deg(state.beta_)
            << ": " << std::fixed << std::setprecision(4)
            << this->dhi()
            << std::endl;

  return true;
}

