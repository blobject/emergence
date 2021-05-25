#include "state.hh"
#include "../util/util.hh"


TEST_CASE("State::State")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  unsigned int num = state.num_;

  REQUIRE(5000 == state.num_);
  REQUIRE(250 == state.width_);
  REQUIRE(250 == state.height_);
  REQUIRE(Approx(Util::deg_to_rad(180)) == state.alpha_);
  REQUIRE(Approx(Util::deg_to_rad(17)) == state.beta_);
  REQUIRE(5.0f == state.scope_);
  REQUIRE(1.3f == state.ascope_);
  REQUIRE(0.67f == state.speed_);
  REQUIRE(0.0f == state.noise_);
  REQUIRE(1.0f == state.prad_);
  REQUIRE(5.0f * 5.0f == state.scope_squared_);
  REQUIRE(1.3f * 1.3f == state.ascope_squared_);
  REQUIRE(num == state.px_.size());
  REQUIRE(num == state.py_.size());
  REQUIRE(num == state.pf_.size());
  REQUIRE(num == state.pc_.size());
  REQUIRE(num == state.ps_.size());
  REQUIRE(num == state.pn_.size());
  REQUIRE(num == state.pl_.size());
  REQUIRE(num == state.pr_.size());
  REQUIRE(num == state.gcol_.size());
  REQUIRE(num == state.grow_.size());
  for (float x : state.px_) {
    REQUIRE((0.0f <= Approx(x) && Approx(x) <= state.width_));
  }
  for (float y : state.py_) {
    REQUIRE((0.0f <= Approx(y) && Approx(y) <= state.height_));
  }
  for (float f : state.pf_) {
    REQUIRE((0.0f <= Approx(f) && Approx(f) <= 360.0f));
  }
  for (int i = 0; i < num; ++i) {
    REQUIRE(Approx(cosf(state.pf_[i])) == state.pc_[i]);
    REQUIRE(Approx(sinf(state.pf_[i])) == state.ps_[i]);
    REQUIRE(0 == state.pn_[i]);
    REQUIRE(0 == state.pl_[i]);
    REQUIRE(0 == state.pr_[i]);
  }
}

TEST_CASE("State::respawn")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  unsigned int num = state.num_;

  state.respawn();
  REQUIRE(num == state.px_.size());
  REQUIRE(num == state.py_.size());
  REQUIRE(num == state.pf_.size());
  REQUIRE(num == state.pc_.size());
  REQUIRE(num == state.ps_.size());
  REQUIRE(num == state.pn_.size());
  REQUIRE(num == state.pl_.size());
  REQUIRE(num == state.pr_.size());
  REQUIRE(num == state.gcol_.size());
  REQUIRE(num == state.grow_.size());
}

TEST_CASE("State::clear")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);

  state.clear();
  REQUIRE(0 == state.px_.size());
  REQUIRE(0 == state.py_.size());
  REQUIRE(0 == state.pf_.size());
  REQUIRE(0 == state.pc_.size());
  REQUIRE(0 == state.ps_.size());
  REQUIRE(0 == state.pn_.size());
  REQUIRE(0 == state.pl_.size());
  REQUIRE(0 == state.pr_.size());
  REQUIRE(0 == state.gcol_.size());
  REQUIRE(0 == state.grow_.size());
}

TEST_CASE("State::change")
{
  auto log = Log(2, QUIET);
  REQUIRE(0 == log.messages_.size());

  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  Stative stative = {
    -1,
    state.num_,
    state.width_,
    state.height_,
    state.alpha_ + 0.1f,
    state.beta_,
    state.scope_,
    state.ascope_,
    state.speed_,
    state.noise_,
    state.prad_,
    state.coloring_
  };
  state.change(stative, false);
  REQUIRE(2 == log.messages_.size());
  REQUIRE("Changed state without respawn." == log.messages_.front().second);

  state.change(stative, QUIET);
  REQUIRE(2 == log.messages_.size());
  REQUIRE("Changed state with respawn." == log.messages_.front().second);

  stative = {
    -1,
    state.num_ + 1,
    state.width_,
    state.height_,
    state.alpha_,
    state.beta_,
    state.scope_,
    state.ascope_,
    state.speed_,
    state.noise_,
    state.prad_,
    state.coloring_
  };
  state.change(stative, QUIET);
  REQUIRE(2 == log.messages_.size());
  REQUIRE("Changed state with respawn." == log.messages_.front().second);
}


TEST_CASE("State::type_name")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);

  REQUIRE("premature spore" == state.type_name(Type::PrematureSpore));
  REQUIRE("mature spore"    == state.type_name(Type::MatureSpore));
  REQUIRE("ring"            == state.type_name(Type::Ring));
  REQUIRE("premature cell"  == state.type_name(Type::PrematureCell));
  REQUIRE("triangle cell"   == state.type_name(Type::TriangleCell));
  REQUIRE("square cell"     == state.type_name(Type::SquareCell));
  REQUIRE("pentagon cell"   == state.type_name(Type::PentagonCell));
  REQUIRE("nutrient"        == state.type_name(Type::Nutrient));
  REQUIRE("cell hull"       == state.type_name(Type::CellHull));
  REQUIRE("cell core"       == state.type_name(Type::CellCore));
}


TEST_CASE("State::dpe")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);

  REQUIRE(5000 == state.num_);
  REQUIRE(250 == state.width_);
  REQUIRE(250 == state.height_);
  REQUIRE(Approx(0.08f) == state.dpe());
}

