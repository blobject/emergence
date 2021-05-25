#include "control.hh"
#include "../util/util.hh"


ExpControl::ExpControl(Log& log, int e)
  : log_(log), experiment_(e)
{
  if (!e) {
    return;
  }
  this->experiment_group_ = 10 <= e ? e / 10 : e;
}


void
ExpControl::message()
{
  int eg = this->experiment_group_;
  int e = this->experiment_;
  std::string message = "Running experiment ";

  if (1 == eg) { message += "1 \"occupancy\": ";
    if      (11 == e) { message += "12 particles."; }
    else if (12 == e) { message += "14 particles."; }
    else if (13 == e) { message += "0.04 dpe."; }
    else if (14 == e) { message += "0.07 dpe."; }
    else if (15 == e) { message += "0.09 dpe."; } }

  else if (2 == eg) { message += "2 \"population\"."; }

  else if (3 == eg) { message += "3 \"heat map\": ";
    if      (31 == e) { message += "nutrients."; }
    else if (32 == e) { message += "premature spore."; }
    else if (33 == e) { message += "mature spore."; }
    else if (34 == e) { message += "ring."; }
    else if (35 == e) { message += "premature cell."; }
    else if (36 == e) { message += "triangle cell."; }
    else if (37 == e) { message += "square cell."; }
    else if (38 == e) { message += "pentagon cell."; } }

  else if (4 == eg) { message += "4 \"survival\": ";
    if      (41 == e) { message += "mature spore survival."; }
    else if (42 == e) { message += "triangle cell survival."; }
    else if (43 == e) { message += "mature spore reproduction."; }
    else if (44 == e) { message += "triangle cell reproduction."; } }

  else if (5 == eg) { message += "5 \"";
    if      (51 == e) { message += "size\": 0.03 dpe."; }
    else if (52 == e) { message += "size\": 0.035 dpe."; }
    else if (53 == e) { message += "size\": 0.04 dpe."; }
    else if (54 == e) { message += "noise\": 0.03 dpe."; }
    else if (55 == e) { message += "noise\": 0.035 dpe."; }
    else if (56 == e) { message += "noise\": 0.04 dpe."; } }

  else if (6 == eg) { message += "6 \"parameter sweep\"."; }

  else if (7 == eg) { message += "7 \"performance\"."; }

  else if (e) {
    this->log_.add(Attn::E, "unknown experiment: " + std::to_string(e));
    return;
  }

  if (e) {
    this->log_.add(Attn::O, message);
  }
}


void
ExpControl::state(State& s)
{
  int e = this->experiment_;
  if (!e) {
    return;
  }

  int eg = this->experiment_group_;

  if (1 == eg) {
    s.width_  = 50;
    s.height_ = 50;
    s.prad_   = 0.5f;
    if      (11 == e) { s.num_ =  12; } // custom placement
    else if (12 == e) { s.num_ =  14; } // custom placement
    else if (13 == e) { s.num_ = 100; } // 0.04 dpe
    else if (14 == e) { s.num_ = 175; } // 0.07 dpe
    else if (15 == e) { s.num_ = 225; } // 0.09 dpe

  } else if (2 == eg) { // 0.08 dpe
    s.num_ = 5000; s.width_ = 250; s.height_ = 250; s.prad_ = 1.0f;

  } else if (3 == eg) { // variable dpe, inj
    s.num_ =    0; s.width_ = 250; s.height_ = 250; s.prad_ = 0.5f;

  } else if (4 == eg) { // variable dpe, inj
    s.num_ = static_cast<unsigned int>(s.width_ * s.height_ * 0.0f);
    s.width_ = 50; s.height_ = 50; s.prad_ = 0.25f;

  } else if (5 == eg) {
    s.width_ = 50; s.height_ = 50; s.prad_ = 0.25f;
    if        (51 == e || 54 == e) { // 0.03 dpe, inj
      s.num_ = static_cast<unsigned int>(s.width_ * s.height_ * 0.03f);
    } else if (52 == e || 55 == e) { // ~0.035 dpe, inj
      s.num_ = static_cast<unsigned int>(s.width_ * s.height_ * 0.035f);
    } else if (53 == e || 56 == e) { // 0.04 dpe, inj
      s.num_ = static_cast<unsigned int>(s.width_ * s.height_ * 0.04f);
    }

  } else if (6 == eg) {
    s.num_    = 1200; // 0.12 dpe, param sweep
    s.width_  = 100;
    s.height_ = 100;
    s.alpha_  = Util::deg_to_rad(180.0f);
    s.beta_   = Util::deg_to_rad(17.0f);
    s.prad_   = 0.5f;

  } else if (7 == eg) {
    if        (71 == e) { s.num_ =   200; s.width_ =  50; s.height_ =  50;
    } else if (72 == e) { s.num_ =   562; s.width_ = 750; s.height_ = 750;
    } else if (73 == e) { s.num_ = 20000; s.width_ = 500; s.height_ = 500;
    } else if (74 == e) { s.num_ =     5; s.width_ =  50; s.height_ =  50; }
  }
}


bool
ExpControl::spawn(State& s)
{
  int e = this->experiment_;
  if (11 != e && 12 != e) {
    return false;
  }

  float center = static_cast<float>(s.width_) / 2.0f;
  float spread = 2.5f;
  float min = center - spread;
  float max = center + spread;
  std::vector<float>& px = s.px_;
  std::vector<float>& py = s.py_;
  for (int i = 0; i < s.num_; ++i) {
    px.push_back(Util::distr(min, max));
    py.push_back(Util::distr(min, max));
  }

  return true;
}


void
ExpControl::control(Control& c)
{
  int e = this->experiment_;
  if (!e) {
    return;
  }

  int eg = this->experiment_group_;
  State& s = c.state_;
  Exp& ex = c.exp_;

  if (1 == eg) { c.duration_ = 150;
    if (15 == e) { c.duration_ = 700; } }

  else if (2 == eg) { c.duration_ = 100000; }

  else if (3 == eg) { c.duration_ = 100;
    if      (31 == e) { c.inject(Type::Nutrient,      true); }
    else if (32 == e) { c.inject(Type::PrematureSpore,true); }
    else if (33 == e) { c.inject(Type::MatureSpore,   true); }
    else if (34 == e) { c.inject(Type::Ring,          true); }
    else if (35 == e) { c.inject(Type::PrematureCell, true); }
    else if (36 == e) { c.inject(Type::TriangleCell,  true); }
    else if (37 == e) { c.inject(Type::SquareCell,    true); }
    else if (38 == e) { c.inject(Type::PentagonCell,  true); } }

  else if (4 == eg) {
    c.duration_ = 25000;
    ex.exp_4_count_ = 1;
    c.dpe_ = static_cast<float>(s.num_) / s.width_ / s.height_;
    if      (41 == e || 43 == e) { c.inject(Type::MatureSpore,  false); }
    else if (42 == e || 44 == e) { c.inject(Type::TriangleCell, false); }
  }

  else if (5 == eg) {
    c.duration_ = 25000;
    ex.exp_5_count_ = 1;
    c.inject(Type::TriangleCell, false); }

  else if (6 == eg) { c.duration_ = 500; }

  // stop after one more tick to gather very last set of data
  if (7 != eg) { ++c.duration_; }
}


void
ExpControl::next(Exp& exp, Control& c)
{
  int e = this->experiment_;
  if (!e) {
    return;
  }

  int eg = this->experiment_group_;
  State& s = c.state_;
  Exp& ex = c.exp_;
  unsigned long tick = c.tick_;

  //ex.do_meta_exp(tick);

  if (1 == eg) {
    if (11 == e ||
        12 == e ||
        13 == e ||
        14 == e) { ex.do_exp_1a(tick); return; }
    if (15 == e) { ex.do_exp_1b(tick); return; } return; }

  if (2 == eg) { ex.do_exp_2(tick); return; }

  if (3 == eg) { ex.do_exp_3(tick); return; }

  if (4 == eg) {
    this->next4_first(c);
    bool done;
    if      (41 == e) { done = ex.do_exp_4a(tick); }
    else if (42 == e) { done = ex.do_exp_4b(tick); }
    else if (43 == e ||
             44 == e) { done = ex.do_exp_4c(tick); }
    if (done) {
      if (!this->next4_iterate(c)) {
        return;
      }
      this->next4_change(c);
    }
    return;
  }

  if (5 == eg) {
    this->next5_first(c);
    bool done;
    if      (51 == e || 52 == e || 53 == e) { done = ex.do_exp_5a(tick); }
    else if (54 == e || 55 == e || 56 == e) { done = ex.do_exp_5b(tick); }
    if (done) {
      float noise = this->next5_iterate(c);
      if (-0.5f > noise) {
        return;
      }
      this->next5_change(c, noise);
    }
    return;
  }

  if (6 == eg) {
    if (ex.do_exp_6(tick)) {
      if (!this->next6_iterate(c)) {
        return;
      }
      this->next6_change(c);
    }
    return;
  }
}


void
ExpControl::next4_first(Control& c)
{
  int e = this->experiment_;
  Exp& ex = c.exp_;

  if (1 == c.tick_ && 0.0001f > c.dpe_) {
    if (41 == e || 42 == e) {
      if (1 < ex.exp_4_count_) {
        std::cout << "\n";
      }
      std::cout << ex.exp_4_count_ << ":" << std::flush;
    }
  }
}


bool
ExpControl::next4_iterate(Control& c)
{
  int e = this->experiment_;
  Exp& ex = c.exp_;
  ex.exp_4_est_done_ = 0;
  ex.exp_4_dbscan_done_ = 0;

  if (0.1001f > c.dpe_ && 40 == e || 41 == e) {
    std::cout << ";";
  }
  if (0.1001f < c.dpe_) {
    ++ex.exp_4_count_;
    c.dpe_ = 0.0f;
  }
  if (10 < ex.exp_4_count_) { // * 10 separate instances
    c.quit();
    return false;
  }
  return true;
}


void
ExpControl::next4_change(Control& c)
{
  int e = this->experiment_;
  State& s = c.state_;
  Stative stative = {
    25001,
    static_cast<int>(s.width_ * s.height_ * c.dpe_),
    s.width_,
    s.height_,
    s.alpha_,
    s.beta_,
    s.scope_,
    s.ascope_,
    s.speed_,
    s.noise_,
    s.prad_,
    s.coloring_
  };
  c.duration_ = -1; // for resetting in change()
  c.change(stative, true);
  if      (41 == e || 43 == e) { c.inject(Type::MatureSpore,  false); }
  else if (42 == e || 44 == e) { c.inject(Type::TriangleCell, false); }
  c.gui_change_ = true; // let UiState reflect true State
}


void
ExpControl::next5_first(Control& c)
{
  int e = this->experiment_;
  Exp& ex = c.exp_;

  if (54 == e || 55 == e || 56 == e) {
    if (1 == c.tick_ && 0.0001f > c.state_.noise_) {
      if (1 < ex.exp_5_count_) {
        std::cout << "\n";
      }
      std::cout << ex.exp_5_count_ << ":" << std::flush;
    }
  }
}


float
ExpControl::next5_iterate(Control& c)
{
  int e = this->experiment_;
  State& s = c.state_;
  Exp& ex = c.exp_;
  ex.exp_5_est_done_ = 0;
  ex.exp_5_dbscan_done_ = 0;
  float noise = 0.0f;

  if (51 == e || 52 == e || 53 == e) {
    ++ex.exp_5_count_;
    if (100 < ex.exp_5_count_) { // * 10 separate instances
      c.quit();
      return -1.0f;
    }
  } else if (54 == e || 55 == e || 56 == e) {
    noise = Util::rad_to_deg(s.noise_);
    if (90.0f <= Util::rad_to_deg(s.noise_)) {
      ++ex.exp_5_count_;
      noise = -5.0f; // will reset noise to 0
    } else {
      std::cout << ";";
    }
    if (100 < ex.exp_5_count_) { // * 10 separate instances
      c.quit();
      return -1.0f;
    }
    noise += 5.0f;
  }
  return noise;
}


void
ExpControl::next5_change(Control& c, float noise)
{
  int e = this->experiment_;
  State& s = c.state_;
  float dpe = 0.03f;
  if      (52 == e || 55 == e) { dpe = 0.035f; }
  else if (53 == e || 56 == e) { dpe = 0.04f; }
  Stative stative = {
    25001,
    static_cast<int>(s.width_ * s.height_ * dpe),
    s.width_,
    s.height_,
    s.alpha_,
    s.beta_,
    s.scope_,
    s.ascope_,
    s.speed_,
    Util::deg_to_rad(noise),
    s.prad_,
    s.coloring_
  };
  c.duration_ = -1; // for resetting in change()
  c.change(stative, true);
  c.inject(Type::TriangleCell, false);
  c.gui_change_ = true; // let UiState reflect true State
}


bool
ExpControl::next6_iterate(Control& c)
{
  State& s = c.state_;
  float alpha = Util::rad_to_deg(s.alpha_);
  float beta = Util::rad_to_deg(s.beta_);

  if (59.5f > beta) {
    s.beta_ = Util::deg_to_rad(beta + 1.0f);
  } else if (179.5f > alpha) {
    s.beta_ = Util::deg_to_rad(-60.0f);
    s.alpha_ = Util::deg_to_rad(alpha + 3.0f);
  } else {
    c.quit();
    return false;
  }
  return true;
}


void
ExpControl::next6_change(Control& c)
{
  State& s = c.state_;
  Stative stative = {
    501,
    1200,
    s.width_,
    s.height_,
    s.alpha_,
    s.beta_,
    s.scope_,
    s.ascope_,
    s.speed_,
    s.noise_,
    s.prad_,
    s.coloring_
  };
  c.duration_ = -1; // for resetting in change()
  c.change(stative, true);
  c.gui_change_ = true; // let UiState reflect true State
}

