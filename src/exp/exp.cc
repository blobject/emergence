#include "exp.hh"
//#include "../util/common.hh"
#include "../util/util.hh"


Exp::Exp(Log& log, State& state)
  : log_(log), state_(state)
{
  this->log_.add(Attn::O, "Starting experiment module.");
}


void
Exp::reset()
{
  this->neighbor_sets_.clear();
  this->cores_.clear();
  this->ambiguous_.clear();
  this->clusters_.clear();
  this->cluster_index_sets_.clear();
  this->random_colors_.clear();
}


void
Exp::coloring(Coloring scheme)
{
  State& state = this->state_;
  float scope = state.scope_;
  std::vector<unsigned int>& pn = state.pn_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;

  if (Coloring::Normal == scheme) {
    for (int p = 0; p < state.num_; ++p) {
      xr[p] = static_cast<float>(pn[p]) / (scope / 1.5f);
      xg[p] = static_cast<float>(pn[p]) / scope;
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
    this->random_color_index_ = 0;
    for (auto& c : this->clusters_) {
      ++this->random_color_index_;
      color = this->random_color();
      for (int p : c) {
        xr[p] = color[0];
        xg[p] = color[1];
        xb[p] = color[2];
      }
    }
    return;
  }

  if (Coloring::Half == scheme) {
    State& state = this->state_;
    unsigned int w = state.width_;
    std::vector<float>& px = state.px_;
    for (int p = 0; p < state.num_; ++p) {
      if (px[p] < w / 2) {
        xr[p] = 1.0f;
        xg[p] = 0.2f;
        xb[p] = 0.2f;
      } else {
        xr[p] = 0.2f;
        xg[p] = 0.2f;
        xb[p] = 1.0f;
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


std::vector<float>
Exp::random_color()
{
  unsigned int index = this->random_color_index_;
  std::vector<std::vector<float>>& colors = this->random_colors_;
  if (index > colors.size()) {
    colors.push_back({
      Util::distribute(0.3f, 1.0f),
      Util::distribute(0.3f, 1.0f),
      Util::distribute(0.3f, 1.0f)
    });
  }
  return colors[index - 1];
}


/* particle hierarchy in clustering
 * --------------------------------
 *
 * world --> grid --> grid unit -.
 *        |                      |
 *        '------------------------> particles
 */


std::string
Exp::cluster()
{
  return "";
}


void
Exp::hdbscan()
{
}


std::string
Exp::cluster2(float radius, unsigned int minpts)
{
  this->reset();
  this->dbscan_categorise(radius, minpts);
  this->dbscan_collect();

  std::stringstream s;
  s << "cores: " << this->cores_.size()
    << "\nambiguous: " << this->ambiguous_.size()
    << "\nCLUSTERS: " << this->clusters_.size() << std::flush;
  //for (auto p : this->cores_) {
  //  s << p << " ";
  //}
  //for (auto p : this->ambiguous_) {
  //  s << p << " ";
  //}

  return s.str();
}


void
Exp::dbscan_categorise(float radius, unsigned int minpts)
{
  this->dbscan_neighborhood2(radius);

  std::unordered_map<int,std::vector<int>>& neighbor_sets = this->neighbor_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<int>& ambiguous = this->ambiguous_;

  std::unordered_map<int,std::vector<int>>::iterator it = neighbor_sets.begin();
  while (it != neighbor_sets.end()) {
    //if (it->second.empty()) {
    //  ++it;
    //  continue;
    //}
    if (minpts > it->second.size()) {
      ambiguous.push_back(it->first);
    } else {
      cores.push_back(it->first);
    }
    ++it;
  }

/*
def:
  N: set of sets of neighbors
input: PTS, RAD, MIN
output: N, CORES, DUNNOS

categorise(PTS, RAD, MIN):
  N := {}
  CORES := {}
  DUNNOS := {}
  foreach P in PTS:
    N_P := neighbors(P, PTS, RAD)
    if |N_P| == 0:
      continue
    add N_P to N
    add P to (|N_P| < MIN ? DUNNOS : CORES)
  return N, CORES, DUNNOS
*/
}


void
Exp::dbscan_neighborhood(float radius)
{
  State& state = this->state_;
  int num = state.num_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::unordered_map<int,std::vector<int>>& n = this->neighbor_sets_;
  float squared = radius * radius;
  float srcx;
  float srcy;
  float dstx;
  float dsty;
  float dx;
  float dy;
  for (int srci = 0; srci < num; ++srci) {
    for (int dsti = srci + 1; dsti < num; ++dsti) {
      srcx = px[srci];
      srcy = py[srci];
      dstx = px[dsti];
      dsty = py[dsti];
      dx = dstx - srcx;
      dy = dsty - srcy;

      if ((dx * dx) + (dy * dy) > squared) {
        continue;
      }

      n[srci].push_back(dsti);
      n[dsti].push_back(srci);
    }
  }
}


void
Exp::dbscan_collect()
{
  std::unordered_map<int, std::vector<int>>& neighbor_sets = this->neighbor_sets_;
  std::vector<int>& cores = this->cores_;
  std::vector<std::unordered_set<int>>& clusters = this->clusters_;
  auto visited = std::unordered_set<int>();
  auto stack = std::unordered_set<int>();
  int q;
  for (int p : cores) {
    if (visited.count(p)) {
      continue;
    }
    stack.insert(p);
    auto cluster = std::unordered_set<int>();
    while (!stack.empty()) {
      q = *stack.begin();
      stack.erase(stack.begin());
      if (cluster.count(q)) {
        continue;
      }
      cluster.insert(q);
      for (int r : neighbor_sets[q]) {
        if (std::find(cores.begin(), cores.end(), r) != cores.end() &&
            !cluster.count(r)) {
          stack.insert(r);
        }
      }
    }
    for (int s : cluster) {
      visited.insert(s);
    }
    clusters.push_back(cluster);
  }
/*
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
---
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


std::string
Exp::inject()
{
  return "";
}


std::string
Exp::densities()
{
  return "";
}


void
Exp::dbscan_neighborhood2(float radius)
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
  std::unordered_map<int,std::vector<int>>& n = this->neighbor_sets_;

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

        n[srci].push_back(dsti);
        n[dsti].push_back(srci);
      }
    }
  }
}

