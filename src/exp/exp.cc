#include "exp.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <algorithm>
#include <iomanip>


Exp::Exp(Log& log, State& state, Proc& proc, bool no_cl)
  : log_(log), state_(state), proc_(proc), no_cl_(no_cl)
{
  // sprite particle positions begin at 0,0 and are proportional to 100x100

  this->sprites_ = {
    {Type::PrematureSpore, gen_sprite({
      0.2864f, 0.294f,  286.124f,
      0.0f,    0.6432f, 214.083f,
      0.7952f, 0.748f,  327.185f,
      0.3136f, 0.7496f, 55.740f,
      0.1688f, 0.2296f, 184.162f,
      0.3064f, 0.7136f, 356.312f,
      0.084f,  0.8044f, 17.696f,
      0.4132f, 0.4516f, 299.425f,
      0.2548f, 0.6476f, 95.066f,
      0.572f,  0.3096f, 14.083f,
      0.3952f, 0.5232f, 273.748f,
      0.0136f, 0.3156f, 122.464f,
      0.288f,  0.0f,    212.428f,
      0.5036f, 0.8844f, 106.633f
    }, 14 * 3)},
    {Type::MatureSpore, gen_sprite({
      0.3756f, 0.4424f, 48.342f,
      0.3968f, 0.1548f, 298.690f,
      0.0828f, 0.5528f, 145.103f,
      0.5512f, 0.2352f, 13.437f,
      0.2340f, 0.7052f, 89.112f,
      0.4712f, 0.0f,    295.822f,
      0.2148f, 0.3696f, 185.900f,
      0.4960f, 0.0184f, 325.073f,
      0.4648f, 0.4132f, 81.238f,
      0.2004f, 0.462f,  123.122f,
      0.4980f, 0.2216f, 320.208f,
      0.4828f, 0.2052f, 315.608f,
      0.3528f, 0.2784f, 277.999f,
      0.1932f, 0.3048f, 211.425f,
      0.3064f, 0.5700f, 73.971f,
      0.024f,  0.4072f, 176.521f,
      0.1844f, 0.4988f, 125.196f,
      0.0f,    0.4776f, 156.439f,
      0.0128f, 0.2516f, 180.004f,
      0.0564f, 0.4696f, 140.666f
    }, 20 * 3)},
    {Type::Ring, gen_sprite({
      0.8616f, 2.4852f, 187.124f,
      0.9364f, 2.1872f, 295.084f,
      1.1144f, 2.3704f, 48.185f,
      1.3416f, 0.1396f, 277.740f,
      0.128f,  2.304f,  148.162f,
      0.1312f, 0.7476f, 187.312f,
      1.4324f, 2.2732f, 260.696f,
      1.7496f, 0.632f,  140.424f,
      1.51f,   0.4844f, 90.066f,
      1.3988f, 0.5688f, 57.083f,
      1.1936f, 0.1912f, 258.748f,
      0.962f,  2.352f,  34.464f,
      0.2892f, 0.4616f, 207.428f,
      1.1616f, 0.0556f, 175.633f,
      0.9872f, 2.5596f, 76.234f,
      0.7172f, 0.3944f, 315.242f,
      1.596f,  2.2468f, 51.356f,
      0.3932f, 0.37f,   187.478f,
      1.5f,    2.2724f, 89.188f,
      1.4912f, 2.4156f, 45.103f,
      2.0688f, 0.774f,  162.208f,
      0.89f,   2.4908f, 146.829f,
      2.3872f, 1.6848f, 216.278f,
      1.3f,    0.0f,    196.410f,
      1.7736f, 0.3048f, 329.741f,
      1.0824f, 2.2236f, 308.877f,
      0.0f,    0.9f,    355.273f,
      1.6296f, 0.3424f, 61.454f,
      1.3f,    2.1736f, 267.142f,
      1.1248f, 2.5524f, 171.674f
    }, 30 * 3)},
    {Type::PrematureCell, gen_sprite({
      0.822f,  2.7192f, 102.124f,
      2.4928f, 2.65f,   206.083f,
      1.3168f, 2.8736f, 261.184f,
      0.978f,  0.358f,  69.739f,
      1.6984f, 1.6004f, 164.161f,
      0.4396f, 1.1308f, 50.696f,
      1.4416f, 2.04f,   160.424f,
      2.45f,   0.9364f, 128.084f,
      0.5108f, 0.4748f, 252.749f,
      2.5216f, 1.2596f, 120.464f,
      0.3468f, 2.0844f, 147.428f,
      2.4288f, 0.9592f, 108.633f,
      1.6188f, 1.7132f, 353.125f,
      2.1644f, 2.992f,  105.214f,
      1.3872f, 2.8528f, 301.837f,
      0.296f,  1.7536f, 189.642f,
      1.0328f, 2.6848f, 339.455f,
      1.6924f, 1.522f,  287.878f,
      2.644f,  1.2196f, 108.058f,
      1.1028f, 2.7944f, 70.158f,
      2.4032f, 0.9156f, 266.730f,
      0.6324f, 2.0196f, 331.309f,
      2.438f,  2.5532f, 269.233f,
      0.4808f, 1.4496f, 45.459f,
      2.7044f, 1.4776f, 137.393f,
      2.4716f, 0.656f,  93.875f,
      1.5116f, 0.1576f, 86.379f,
      0.4088f, 1.68f,   5.545f,
      1.2392f, 0.1572f, 327.751f,
      0.4668f, 1.71f,   8.065f,
      2.6248f, 1.5836f, 138.623f,
      2.36f,   2.75f,   234.053f,
      2.568f,  1.1848f, 205.344f,
      0.0f,    1.4316f, 30.125f,
      2.8484f, 1.0512f, 331.035f,
      0.3788f, 0.9836f, 152.064f,
      2.7384f, 1.7812f, 260.862f,
      1.0116f, 0.0f,    320.082f
    }, 38 * 3)},
    {Type::TriangleCell, gen_sprite({
      1.4532f, 2.83f,   0.516f,
      0.0f,    1.464f,  185.466f,
      3.2788f, 2.5924f, 14.954f,
      1.7836f, 2.9948f, 10.084f,
      1.5716f, 3.1608f, 93.85f,
      1.1048f, 0.5356f, 355.234f,
      0.386f,  2.638f,  133.442f,
      0.7872f, 0.7324f, 60.218f,
      2.982f,  0.9692f, 315.241f,
      2.5432f, 2.9588f, 34.492f,
      3.0352f, 2.1508f, 90.069f,
      1.0672f, 2.6896f, 331.227f,
      0.642f,  2.4468f, 308.996f,
      1.7084f, 1.7504f, 150.401f,
      0.2692f, 2.5036f, 174.981f,
      1.8036f, 3.364f,  127.082f,
      3.052f,  1.0724f, 341.426f,
      2.4028f, 0.0f,    278.286f,
      0.9252f, 2.8636f, 121.066f,
      0.8348f, 2.448f,  349.447f,
      2.758f,  0.4008f, 270.608f,
      1.5164f, 1.7144f, 165.184f,
      1.6676f, 1.8684f, 45.607f,
      1.4772f, 0.458f,  347.785f,
      1.7236f, 0.1068f, 244.939f,
      0.0244f, 0.9264f, 213.885f,
      2.7228f, 2.2672f, 193.488f,
      0.1664f, 2.3816f, 30.367f,
      3.116f,  1.7104f, 84.282f,
      2.1956f, 0.206f,  28.476f,
      2.752f,  0.8244f, 133.385f,
      0.194f,  1.0212f, 229.126f,
      3.0268f, 1.5008f, 23.606f,
      2.77f,   1.418f,  171.83f,
      2.174f,  0.2152f, 309.684f,
      1.2928f, 2.8012f, 208.041f,
      0.8148f, 0.4888f, 184.779f,
      0.4036f, 0.824f,  234.282f,
      2.928f,  1.3548f, 240.642f,
      1.636f,  1.3484f, 250.784f
    }, 40 * 3)},
    {Type::TriangleCell, gen_sprite({
      2.6984f, 4.078f,  247.125f,
      2.962f,  1.4632f, 238.082f,
      1.2532f, 2.9716f, 333.185f,
      2.8352f, 0.8268f, 341.738f,
      3.8352f, 3.0548f, 0.162f,
      0.7692f, 0.6936f, 226.311f,
      4.266f,  1.11f,   57.695f,
      3.6012f, 3.0536f, 268.426f,
      1.5632f, 1.636f,  246.065f,
      0.0668f, 1.4884f, 168.748f,
      4.156f,  1.166f,  343.463f,
      2.442f,  0.0f,    237.429f,
      0.0f,    2.5024f, 223.632f,
      1.2016f, 0.2632f, 128.125f,
      1.838f,  3.8044f, 204.213f,
      1.3412f, 3.2164f, 104.352f,
      3.5328f, 0.956f,  10.636f,
      0.22f,   1.378f,  220.214f,
      2.8472f, 0.1468f, 221.936f,
      1.7508f, 0.1548f, 187.838f,
      2.6772f, 0.116f,  334.643f,
      3.5264f, 2.0096f, 223.639f,
      0.8892f, 1.31f,   33.455f,
      1.3644f, 3.0836f, 314.850f,
      0.9496f, 0.7036f, 221.471f,
      1.7256f, 3.9136f, 84.753f,
      4.0116f, 3.0484f, 7.876f,
      3.7336f, 3.2644f, 99.757f,
      2.7296f, 0.2624f, 187.059f,
      1.9748f, 1.2112f, 218.345f,
      1.8884f, 2.8988f, 44.159f,
      3.1212f, 0.944f,  224.730f,
      3.264f,  1.0728f, 96.363f,
      1.2264f, 3.2588f, 161.694f,
      1.036f,  0.7092f, 218.308f,
      1.8116f, 3.9484f, 109.068f,
      3.2052f, 3.3496f, 214.232f,
      0.3688f, 2.7092f, 231.172f,
      0.8788f, 1.05f,   131.459f,
      3.2148f, 3.4364f, 71.392f,
      1.0504f, 3.0032f, 210.875f,
      1.422f,  0.7868f, 27.377f,
      2.2736f, 4.1912f, 236.895f,
      4.4296f, 1.634f,  162.545f,
      2.0584f, 2.7852f, 349.234f,
      4.4328f, 1.644f,  180.751f,
      0.5672f, 1.1748f, 177.064f,
      3.9368f, 3.0208f, 8.623f,
      3.472f,  3.194f,  280.385f,
      3.242f,  0.5908f, 269.272f,
      4.2748f, 0.9156f, 18.052f,
      0.982f,  0.992f,  186.343f,
      3.7876f, 2.7644f, 321.125f,
      4.5436f, 2.6188f, 27.035f,
      0.3876f, 2.6396f, 45.065f,
      4.5264f, 2.3212f, 100.754f,
      3.392f,  3.4752f, 292.861f,
      3.166f,  1.7056f, 309.752f,
      0.9008f, 0.8304f, 273.082f,
      1.1932f, 3.056f,  338.375f
    }, 60 * 3)},
    {Type::PentagonCell, gen_sprite({
      1.7989f, 3.8219f, 177.803f,
      0.1499f, 0.7716f, 291.991f,
      2.2213f, 3.3414f, 325.542f,
      0.981f,  0.7541f, 143.343f,
      0.5295f, 2.903f,  296.777f,
      3.9997f, 1.0573f, 280.155f,
      0.9086f, 3.5979f, 165.413f,
      2.6673f, 0.0452f, 163.035f,
      0.3288f, 2.3692f, 34.826f,
      0.3921f, 2.7499f, 299.219f,
      0.8205f, 1.7633f, 309.215f,
      1.5393f, 0.522f,  102.974f,
      4.7801f, 3.0543f, 150.049f,
      0.6324f, 3.4314f, 312.194f,
      3.2205f, 0.0f,    151.37f,
      0.0f,    2.2458f, 193.951f,
      0.7883f, 3.9779f, 74.3767f,
      0.9308f, 0.9845f, 88.454f,
      4.2745f, 1.2488f, 274.492f,
      3.7773f, 3.703f,  104.889f,
      4.5261f, 1.9871f, 349.692f,
      2.0369f, 3.5057f, 178.78f,
      3.0573f, 3.989f,  28.2423f,
      4.1085f, 1.3504f, 124.645f,
      2.1273f, 1.1918f, 203.023f,
      2.0353f, 0.5466f, 100.377f,
      4.3877f, 1.3726f, 299.715f,
      1.4597f, 0.534f,  120.814f,
      1.7429f, 3.9121f, 335.511f,
      3.2593f, 4.24f,   142.513f,
      4.3833f, 1.5508f, 156.496f,
      3.8617f, 1.7982f, 303.564f,
      1.8701f, 3.9085f, 73.2385f,
      2.0629f, 0.3492f, 129.633f,
      3.8301f, 3.6288f, 257.273f,
      3.7181f, 1.1025f, 125.117f,
      0.692f,  2.589f,  337.344f,
      3.8785f, 3.7668f, 92.1357f,
      0.0903f, 2.348f,  213.266f,
      2.1877f, 3.872f,  256.09f,
      0.1407f, 2.8387f, 190.997f,
      4.0393f, 1.3159f, 154.397f,
      4.1745f, 3.6342f, 34.2524f,
      2.7485f, 2.3562f, 54.0567f,
      2.1897f, 2.0942f, 227.375f,
      0.3435f, 2.0516f, 54.1957f,
      3.9845f, 3.8348f, 23.6461f,
      3.5413f, 0.487f,  46.7108f,
      0.4122f, 2.651f,  325.745f,
      4.4049f, 3.3572f, 16.937f,
      1.9817f, 0.3721f, 107.526f,
      3.9805f, 3.4946f, 275.841f,
      2.4749f, 2.0687f, 29.9617f,
      0.5112f, 0.8258f, 34.881f,
      1.5569f, 3.7072f, 346.838f,
      1.5113f, 0.6524f, 73.6977f,
      0.3139f, 1.1176f, 214.387f,
      4.2005f, 3.5708f, 81.3016f,
      4.9589f, 2.3758f, 297.52f,
      1.4169f, 1.8736f, 170.339f,
      1.21f,   0.6984f, 87.1549f,
      4.3585f, 3.1502f, 178.306f,
      4.0097f, 3.6533f, 0.876778f,
      4.9013f, 2.1788f, 127.617f,
      3.7909f, 1.247f,  132.64f,
      0.3802f, 1.7957f, 288.839f,
      2.2209f, 1.9383f, 227.456f,
      1.7237f, 3.5908f, 277.55f,
      4.4189f, 2.8839f, 356.809f
    }, 69 * 3)}
  };

  this->sprite_index_ = 0;

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
  this->reset_color(); // reset coloring last
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
}


void
Exp::reset_inject()
{
  this->sprite_index_ = 0;
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
  this->cluster_type();
}


bool
Exp::inject(Type type, float dpe)
{
  State& state = this->state_;
  SpritePts ps = this->sprites_[type];
  float w = static_cast<float>(state.width_);
  float h = static_cast<float>(state.height_);
  unsigned int size = ps.size();

  // fail if insufficient num
  unsigned int num = static_cast<unsigned int>(w * h * dpe);
  if (num < size) {
    return false;
  }

  // fix dpe by resizing
  unsigned int n_stride = state.n_stride_;
  if (num < state.num_) {
    // truncate particle vectors
    state.px_.resize(num);
    state.py_.resize(num);
    state.pf_.resize(num);
    state.pc_.resize(num);
    state.ps_.resize(num);
    state.pn_.resize(num);
    state.pl_.resize(num);
    state.pr_.resize(num);
    state.pan_.resize(num);
    state.pls_.resize(n_stride * num);
    state.prs_.resize(n_stride * num);
    state.pld_.resize(n_stride * num);
    state.prd_.resize(n_stride * num);
    state.pt_.resize(num);
    state.xr_.resize(num);
    state.xg_.resize(num);
    state.xb_.resize(num);
    state.xa_.resize(num);
  } else if (num > state.num_) {
    // pad with random particles
    for (int i = state.num_; i < num; ++i) {
      state.px_.push_back(Util::distr(0.0f, w));
      state.py_.push_back(Util::distr(0.0f, h));
      state.pf_.push_back(Util::distr(0.0f, TAU));
      state.pc_.push_back(cosf(state.pf_[i]));
      state.ps_.push_back(sinf(state.pf_[i]));
      state.pn_.push_back(0);
      state.pl_.push_back(0);
      state.pr_.push_back(0);
      state.pan_.push_back(0);
      for (int j = 0; j < n_stride; ++j) { state.pls_.push_back(-1); }
      for (int j = 0; j < n_stride; ++j) { state.prs_.push_back(-1); }
      for (int j = 0; j < n_stride; ++j) { state.pld_.push_back(-1.0f); }
      for (int j = 0; j < n_stride; ++j) { state.prd_.push_back(-1.0f); }
      state.pt_.push_back(Type::None);
      state.xr_.push_back(1.0f);
      state.xg_.push_back(1.0f);
      state.xb_.push_back(1.0f);
      state.xr_.push_back(0.5f);
    }
  }
  state.num_ = num;

  // inject: maintain an index that starts at 0 and points to the end of the
  //         last injected sprite, circling around if it overshoots num
  if (num <= this->sprite_index_ + size) {
    this->sprite_index_ = 0;
  }
  float dist_x = Util::distr(0.0f, w);
  float dist_y = Util::distr(0.0f, h);
  float x;
  float y;
  unsigned int si = 0;
  unsigned int istride;
  for (int i = this->sprite_index_; i < this->sprite_index_ + size; ++i) {
    SpritePt& p = ps[si];
    ++si;
    istride = n_stride * si;
    x = std::get<0>(p) + dist_x; if (w <= x) { x -= w; }
    y = std::get<1>(p) + dist_y; if (h <= y) { y -= h; }
    state.px_[i] = x;
    state.py_[i] = y;
    state.pf_[i] = std::get<2>(p);
    state.pc_[i] = std::get<3>(p);
    state.ps_[i] = std::get<4>(p);
    state.pn_[i] = 0;
    state.pl_[i] = 0;
    state.pr_[i] = 0;
    state.pan_[i] = 0;
    for (int j = 0; j < n_stride; ++j) { state.pls_[istride + j] = -1; }
    for (int j = 0; j < n_stride; ++j) { state.prs_[istride + j] = -1; }
    for (int j = 0; j < n_stride; ++j) { state.pld_[istride + j] = -1.0f; }
    for (int j = 0; j < n_stride; ++j) { state.prd_[istride + j] = -1.0f; }
    state.pt_[i] = type;
    state.xr_[i] = 1.0f;
    state.xg_[i] = 1.0f;
    state.xb_[i] = 1.0f;
    state.xa_[i] = 1.0f;
  }
  this->sprite_x_ = dist_x;
  this->sprite_y_ = dist_y;
  this->sprite_index_ += size;

  return true;
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


bool
Exp::is_cluster_type(TypeBit target, std::set<int>& cluster)
{
  std::vector<Type>& pt = this->state_.pt_;
  unsigned int threshold = cluster.size() / 2; // at least 50%
  unsigned int count = 0;
  Type t;
  TypeBit b = TypeBit::Nutrient;

  // TODO: room for optimisation (return earlier)
  for (int p : cluster) {
    if (threshold < count) {
      return true;
    }
    t = pt[p];
    if      (Type::CellHull       == t) { b = TypeBit::CellHull; }
    else if (Type::CellCore       == t) { b = TypeBit::CellCore; }
    else if (Type::MatureSpore    == t) { b = TypeBit::MatureSpore; }
    else if (Type::PrematureSpore == t) { b = TypeBit::PrematureSpore; }
    else if (Type::Nutrient       == t) { b = TypeBit::Nutrient; }
    if (static_cast<int>(target) & static_cast<int>(b)) {
      ++count;
    }
  }

  return false;
}


void
Exp::cluster_type() {
  std::vector<std::set<int>>& clusters = this->clusters_;
  TypeBit cell = static_cast<TypeBit>(static_cast<int>(Type::CellHull)
                                      | static_cast<int>(Type::CellCore));

  for (int i = 0; i < clusters.size(); ++i) {
    if (this->is_cluster_type(TypeBit::MatureSpore, clusters[i])) {
      this->spore_clusters_.insert(i);
      continue;
    }
    if (this->is_cluster_type(cell, clusters[i])) {
      this->cell_clusters_.insert(i);
    }
  }
}


std::vector<std::tuple<float,float,float,float,float>>
Exp::gen_sprite(std::vector<float> xyf, unsigned int num)
{
  State& state = this->state_;
  float scale = std::min(state.width_, state.height_) / 100.0f;
  float f;
  std::vector<std::tuple<float,float,float,float,float>> ps;

  for (int i = 0; i < num; i += 3) {
    f = Util::deg_to_rad(xyf[i + 2]);
    ps.push_back({scale * xyf[i], scale * xyf[i + 1], f, cosf(f), sinf(f)});
  }

  this->sprite_scale_ = scale;

  return ps;
}


void
Exp::brief_meta_exp(unsigned int tick) {
  if (tick % 50) {
    return;
  }
  std::cout << tick << ": "
            << this->magentas_ << " magenta(mature_spore), "
            << this->blues_ << " blue(cell_hull), "
            << this->yellows_ << " yellow(cell_core)"
            << std::endl;
}


void
Exp::brief_exp_1a(unsigned int tick) {
  if (0 == tick || 149 == tick) {
    float radius = 2.0f * this->state_.scope_;
    unsigned int minpts = 14;
    this->cluster(radius, minpts);
    this->nearest_neighbor_dists_.clear();
    this->nearest_neighbor_dists();
    std::cout << tick << ":\n";
    for (float dist : this->nearest_neighbor_dists_) {
      std::cout << dist << " ";
    }
    std::cout << std::endl;
  }
}


void
Exp::brief_exp_1b(unsigned int tick) {
  if (  0 == tick ||
       60 == tick ||
       90 == tick ||
      180 == tick ||
      400 == tick ||
      699 == tick)
  {
    float radius = 2.0f * this->state_.scope_;
    unsigned int minpts = 14;
    this->cluster(radius, minpts);
    this->nearest_neighbor_dists_.clear();
    this->nearest_neighbor_dists();
    std::cout << tick << ":\n";
    for (float dist : this->nearest_neighbor_dists_) {
      std::cout << dist << " ";
    }
    std::cout << std::endl;
  }
}


void
Exp::brief_exp_2(unsigned int tick) {
  if (tick % 100) {
    return;
  }
  float radius = 0.75f * this->state_.scope_;
  unsigned int minpts = 14;
  this->cluster(radius, minpts);
  std::cout << std::fixed << std::setprecision(2)
            << tick << ": "
            << this->magentas_ << " mature_spores, "
            << this->blues_ << " cell_hulls, "
            << this->yellows_ << " cell_cores "
            << "(dbscan " << radius << "," << minpts << ": "
            << this->clusters_.size() << " clusters, "
            << this->cell_clusters_.size() << " cells, "
            << this->spore_clusters_.size() << " spores, "
            << this->cores_.size() << " cores, "
            << this->vague_.size() << " vagues, "
            << this->state_.num_ << " noise)"
            << std::endl;
}

