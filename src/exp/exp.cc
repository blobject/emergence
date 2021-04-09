#include "exp.hh"
#include "../util/common.hh"
#include "../util/util.hh"
#include <algorithm>


Exp::Exp(Log& log, State& state)
  : log_(log), state_(state)
{
  this->log_.add(Attn::O, "Starting experiment module.");

  this->sprites_ = {
    { Type::PrematureSpore, gen_sprite({
      0.0f,  0.0f,   0.0f,  0.25f,
      0.25f, 0.0f,   0.25f, 0.25f,
      0.5f,  0.0f,   0.5f,  0.25f,
      0.0f,  0.5f,   0.25f, 0.5f,
      0.5f,  0.5f,   0.25f, 0.75f,
      0.75f, 0.25f,  0.75f, 0.75f,
      0.75f, 0.75f,  0.75f, 0.75f
    }, 14) },
    { Type::MatureSpore, gen_sprite({
                   0.2f, 0.0f,  0.3f, 0.0f,
      0.1f, 0.1f,  0.2f, 0.1f,  0.3f, 0.1f,  0.4f, 0.1f,
      0.1f, 0.2f,  0.2f, 0.2f,  0.3f, 0.2f,  0.4f, 0.2f,
      0.1f, 0.3f,  0.2f, 0.3f,  0.3f, 0.3f,  0.4f, 0.3f,
      0.1f, 0.4f,  0.2f, 0.4f,  0.3f, 0.4f,  0.4f, 0.4f,
                   0.2f, 0.5f,  0.3f, 0.5f
    }, 20) },
    { Type::Ring, gen_sprite({
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
      0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f
    }, 40) }
  };

  // premature cell (?)
  std::vector<float>xy = {
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f
  };
  this->sprites_.insert({Type::PrematureCell, gen_sprite(xy, xy.size())});

  // triangle cell (?)
  xy = {
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f
  };
  this->sprites_.insert({Type::TriangleCell, gen_sprite(xy, xy.size())});

  // square cell (?)
  xy = {
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f
  };
  this->sprites_.insert({Type::SquareCell, gen_sprite(xy, xy.size())});

  // pentagon cell (?)
  xy = {
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f
  };
  this->sprites_.insert({Type::PentagonCell, gen_sprite(xy, xy.size())});

  this->sprite_index_ = 0;
}


void
Exp::type()
{
  State& state = this->state_;
  std::vector<Type>& pt = state.pt_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<float>& pnd = state.pnd_;
  unsigned int stride = state.n_stride_;
  float alt_radius = 1.3f;
  float alt_dist = alt_radius * alt_radius;
  unsigned int n;

  for (unsigned int p = 0; p < state.num_; ++p) {
    n = pn[p];
    if (15 < n && 15 < alt_neighborhood(pnd, p, stride, alt_dist)) {
      pt[p] = Type::MatureSpore;
      continue;
    }
    if (15 <  n && n <= 35) { pt[p] = Type::CellHull; continue; }
    if (35 <  n)            { pt[p] = Type::CellCore; continue; }
    if (13 <= n && n <= 15) { pt[p] = Type::PrematureSpore; continue;  }
    pt[p] = Type::Nutrient;
  }
}


std::string
Exp::type_name(Type type)
{
  return TypeNames[static_cast<int>(type)];
}


void
Exp::coloring(Coloring scheme)
{
  State& state = this->state_;
  std::vector<Type>& pt = state.pt_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;

  if (Coloring::Original == scheme) {
    float alt_radius = 1.3f;
    std::vector<float>& pnd = state.pnd_;
    unsigned int stride = state.n_stride_;
    float alt_dist = alt_radius * alt_radius;

    for (unsigned int p = 0; p < state.num_; ++p) {
      if (Type::MatureSpore == pt[p]) {
        // magenta
        xr[p] = 0.8f;
        xg[p] = 0.2f;
        xb[p] = 0.4f;
        continue;
      }
      if (Type::CellHull == pt[p]) {
        // blue
        xr[p] = 0.2f;
        xg[p] = 0.4f;
        xb[p] = 0.8f;
        continue;
      }
      if (Type::CellCore == pt[p]) {
        // yellow
        xr[p] = 0.8f;
        xg[p] = 0.8f;
        xb[p] = 0.0f;
        continue;
      }
      if (Type::PrematureSpore == pt[p]) {
        // brown
        xr[p] = 0.4f;
        xg[p] = 0.2f;
        xb[p] = 0.1f;
        continue;
      }
      // green (nutrient)
      xr[p] = 0.2f;
      xg[p] = 0.6f;
      xb[p] = 0.0f;
    }
    return;
  }

  float scope = state.scope_;

  if (Coloring::Dynamic == scheme) {
    for (int p = 0; p < state.num_; ++p) {
      xr[p] = pn[p] / (scope / 1.5f);
      xg[p] = pn[p] / scope;
      xb[p] = 0.7f;
    }
    return;
  }

  if (Coloring::Cluster == scheme) {
    for (int p = 0; p < state.num_; ++p) {
      xr[p] = 0.2f;
      xg[p] = 0.2f;
      xb[p] = 0.2f;
    }
    std::vector<float> color = {};
    this->palette_index_ = 0;
    for (std::unordered_set<int>& c : this->clusters_) {
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
      }
    }
    return;
  }

  unsigned int threshold = 0;
  if      (Coloring::Density10 == scheme) { threshold = 10; }
  else if (Coloring::Density15 == scheme) { threshold = 15; }
  else if (Coloring::Density20 == scheme) { threshold = 20; }
  else if (Coloring::Density25 == scheme) { threshold = 25; }
  else if (Coloring::Density30 == scheme) { threshold = 30; }
  for (int p = 0; p < state.num_; ++p) {
    if (threshold > pn[p]) {
      xr[p] = 0.2f;
      xg[p] = 0.2f;
      xb[p] = 0.2f;
    } else {
      xr[p] = 1.0f;
      xg[p] = 1.0f;
      xb[p] = 1.0f;
    }
  }
}


void
Exp::reset_exp()
{
  this->reset_cluster();
  this->reset_inject();
}


void
Exp::reset_cluster()
{
  this->neighbor_sets_.clear();
  this->cores_.clear();
  this->vague_.clear();
  this->clusters_.clear();
  this->palette_.clear();
  this->cell_clusters_ = 0;
  this->spore_clusters_ = 0;
}


void
Exp::reset_inject()
{
  this->sprite_index_ = 0;
}


void
Exp::cluster(float radius, unsigned int minpts)
{
  this->reset_cluster();
  this->dbscan_categorise(radius, minpts);
  this->dbscan_collect();
  this->cluster_type_count();
}


void
Exp::inject(Type type, float dpe)
{
  State& state = this->state_;
  SpritePts ps = this->sprites_[type];
  float w = static_cast<float>(state.width_);
  float h = static_cast<float>(state.height_);
  unsigned int n_stride = state.n_stride_;
  unsigned int size = ps.size();
  float place_x = Util::dist(0.0f, w);
  float place_y = Util::dist(0.0f, h);
  float scale = std::min(w, h) / 100.0f;

  // scale and translate sprite
  float x;
  float y;
  for (int i = 0; i < size; ++i) {
    SpritePt& p = ps[i];
    x = scale * std::get<0>(p) + place_x; if (w < x) { x -= w; }
    y = scale * std::get<1>(p) + place_y; if (h < y) { y -= h; }
    ps[i] = { x, y, std::get<2>(p), std::get<3>(p), std::get<4>(p) };
  }

  // fix dpe by resizing
  unsigned int num = static_cast<unsigned int>(w * h * dpe);
  if (num < state.num_) {
    // truncate particle vectors
    state.px_.resize(num);
    state.py_.resize(num);
    state.pf_.resize(num);
    state.pc_.resize(num);
    state.ps_.resize(num);
    state.pn_.resize(num);
    state.pnd_.resize(n_stride * num);
    state.pl_.resize(num);
    state.pr_.resize(num);
    state.pt_.resize(num);
    state.xr_.resize(num);
    state.xg_.resize(num);
    state.xb_.resize(num);
  } else if (num > state.num_) {
    // pad with random particles
    for (int i = state.num_; i < num; ++i) {
      state.px_.push_back(Util::dist(0.0f, w));
      state.py_.push_back(Util::dist(0.0f, h));
      state.pf_.push_back(Util::dist(0.0f, TAU));
      state.pc_.push_back(cosf(state.pf_[i]));
      state.ps_.push_back(sinf(state.pf_[i]));
      state.pn_.push_back(0);
      for (int j = 0; j < n_stride; ++j) {
        state.pnd_.push_back(-1.0f);
      }
      state.pl_.push_back(0);
      state.pr_.push_back(0);
      state.pt_.push_back(Type::None);
      state.xr_.push_back(0.5f);
      state.xg_.push_back(0.5f);
      state.xb_.push_back(0.5f);
    }
  }
  state.num_ = num;

  // inject: maintain an index that starts at 0 and points to the end of the
  //         last injected sprite, circling around if it overshoots num
  if (num <= this->sprite_index_ + size) {
    this->sprite_index_ -= num - size;
  }
  unsigned int pi = 0;
  for (int i = this->sprite_index_; i < this->sprite_index_ + size; ++i) {
    SpritePt& p = ps[pi];
    ++pi;
    state.px_[i] = std::get<0>(p);
    state.py_[i] = std::get<1>(p);
    state.pf_[i] = std::get<2>(p);
    state.pc_[i] = std::get<3>(p);
    state.ps_[i] = std::get<4>(p);
    state.pn_[i] = 0;
    for (int j = 0; j < n_stride; ++j) {
      state.pnd_[n_stride * i + j] = -1.0f;
    }
    state.pl_[i] = 0;
    state.pr_[i] = 0;
    state.pt_[i] = type;
    state.xr_[i] = 1.0f;
    state.xg_[i] = 1.0f;
    state.xb_[i] = 1.0f;
  }
  this->sprite_index_ += size;
  this->sprite_scale_ = scale;
  this->sprite_x_ = place_x;
  this->sprite_y_ = place_y;
}


unsigned int
Exp::alt_neighborhood(std::vector<float>& pnd, unsigned int p,
                      unsigned int stride, float alt_radius_squared)
{
  unsigned int count = 0;
  float d;

  for (int i = 0; i < stride; ++i) {
    d = pnd[stride * p + i];
    if (0 > d) {
      break;
    }
    if (alt_radius_squared >= d) {
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
      Util::dist(0.3f, 1.0f),
      Util::dist(0.3f, 1.0f),
      Util::dist(0.3f, 1.0f)
    });
  }

  return colors[index - 1];
}


void
Exp::dbscan_categorise(float radius, unsigned int minpts)
{
  this->dbscan_neighborhood(radius);

  std::unordered_map<int,std::vector<int>>& ns = this->neighbor_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<int>& vague = this->vague_;

  std::unordered_map<int,std::vector<int>>::iterator it = ns.begin();
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
  std::unordered_map<int, std::vector<int>>& ns = this->neighbor_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<std::unordered_set<int>>& clusters = this->clusters_;
  auto visited = std::unordered_set<int>();
  auto stack = std::unordered_set<int>();
  int q;

  for (int p : cores) {
    if (0 < visited.count(p)) {
      continue;
    }
    stack.insert(p);
    auto cluster = std::unordered_set<int>();
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
Exp::dbscan_neighborhood(float radius)
{
  State& state = this->state_;
  int num = state.num_;
  float w = state.width_;
  float h = state.height_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  int cols = 1; if (radius < w) { cols = floor(w / radius); }
  int rows = 1; if (radius < h) { rows = floor(h / radius); }
  unsigned int grid_size = cols * rows;
  auto unflat = std::vector<std::vector<int>>(grid_size); // signed!
  float unit_w = w / cols;
  float unit_h = h / rows;
  std::vector<int> gcol;
  std::vector<int> grow;
  std::unordered_map<int,std::vector<int>>& ns = this->neighbor_sets_;

  int col;
  int row;
  for (int i = 0; i < num; ++i) {
    col = floor(px[i] / unit_w); if (col >= cols) { col = cols - 1; }
    row = floor(py[i] / unit_h); if (row >= rows) { row = rows - 1; }
    gcol.push_back(col);
    grow.push_back(row);
    unflat[cols * row + col].push_back(i);
  }

  unsigned int stride = 0;
  unsigned int count = 0;
  for (unsigned int i = 0; i < grid_size; ++i) {
    count = unflat[i].size();
    if (count > stride) {
      stride = count;
    }
  }

  int grid[cols * rows * stride];
  int grid_index = 0;
  for (std::vector<int> unit : unflat) {
    count = 0;
    for (int p : unit) {
      grid[grid_index++] = p;
      ++count;
    }
    while (count < stride) {
      grid[grid_index++] = -1;
      ++count;
    }
  }

  int c;
  int cc;
  int r;
  int rr;
  bool cunder;
  bool cover;
  bool runder;
  bool rover;
  bool cu;
  bool co;
  bool ru;
  bool ro;
  float srcx;
  float srcy;
  float dstx;
  float dsty;
  float dx;
  float dy;
  float squared = radius * radius;
  int dsti;
  for (int srci = 0; srci < num; ++srci) {
    col = gcol[srci];
    row = grow[srci];
    c = col - 1;
    cc = col + 1;
    r = row - 1;
    rr = row + 1;
    cunder = false;
    cover = false;
    runder = false;
    rover = false;
    if      (col == 0)        { cunder = true; c = cols - 1; }
    else if (col == cols - 1) { cover  = true; cc = 0; }
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
    // for every unit in the vicinity (grid neighborhood)
    for (unsigned int v = 0; v < 54; v += 6) {
      // for each particle index within the unit
      for (unsigned int p = 0; p < stride; ++p) {
        dsti = grid[cols * (vic[v + 1] * stride) + (vic[v] * stride) + p];
        // avoid grid padding area (meaning no particle left in that unit)
        if (dsti == -1) {
          break;
        }
        // avoid redundant calculations
        if (srci <= dsti) {
          continue;
        }

        cu = vic[v + 2];
        co = vic[v + 3];
        ru = vic[v + 4];
        ro = vic[v + 5];
        srcx = px[srci];
        srcy = py[srci];
        dstx = px[dsti];
        dsty = py[dsti];
        dx = dstx - srcx; if (cu) { dx -= w; } else if (co) { dx += w; }
        dy = dsty - srcy; if (ru) { dy -= h; } else if (ro) { dy += h; }

        // ignore comparisons outside the vicinity scope
        if ((dx * dx) + (dy * dy) > squared) {
          continue;
        }

        ns[srci].push_back(dsti);
        ns[dsti].push_back(srci);
      }
    }
  }
}


bool
Exp::is_cluster_type(TypeBit target, std::unordered_set<int>& cluster)
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
Exp::cluster_type_count() {
  std::vector<std::unordered_set<int>>& clusters = this->clusters_;
  TypeBit cell = static_cast<TypeBit>(static_cast<int>(Type::CellHull)
                                      | static_cast<int>(Type::CellCore));

  for (std::unordered_set<int>& cluster : clusters) {
    if (this->is_cluster_type(TypeBit::MatureSpore, cluster)) {
      ++this->spore_clusters_;
      continue;
    }
    if (this->is_cluster_type(cell, cluster)) {
      ++this->cell_clusters_;
    }
  }
}


std::vector<std::tuple<float,float,float,float,float>>
Exp::gen_sprite(std::vector<float> xy, unsigned int num)
{
  float f;
  std::vector<std::tuple<float,float,float,float,float>> ps;

  for (int i = 0; i < num; i += 2) {
    f = Util::dist<float>(0.0f, TAU);
    ps.push_back({xy[i], xy[i + 1], f, cosf(f), sinf(f)});
  }

  return ps;
}

