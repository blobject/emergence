#include "../src/state/state.hh"
#include "../src/util/util.hh"


TEST_CASE("State::State")
{
  Log log = Log(1);
  State state = State(log);
  unsigned int num = state.num_;
  REQUIRE(5000 == state.num_);
  REQUIRE(250 == state.width_);
  REQUIRE(250 == state.height_);
  REQUIRE(Approx(Util::deg_to_rad(180)) == state.alpha_);
  REQUIRE(Approx(Util::deg_to_rad(17)) == state.beta_);
  REQUIRE(5.0f == state.scope_);
  REQUIRE(1.3f == state.ascope_);
  REQUIRE(0.67f == state.speed_);
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
}

TEST_CASE("State::respawn")
{
  Log log = Log(1);
  State state = State(log);
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
  Log log = Log(1);
  State state = State(log);
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
  Log log = Log(2);
  REQUIRE(0 == log.messages_.size());

  State state = State(log);
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
    state.prad_,
    state.coloring_
  };
  state.change(stative, false);
  REQUIRE(1 == log.messages_.size());
  REQUIRE("Changed state without respawn." == log.messages_.front().second);

  state.change(stative, true);
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
    state.prad_,
    state.coloring_
  };
  state.change(stative, true);
  REQUIRE(3 == log.messages_.size());
  REQUIRE("Changed state with respawn." == log.messages_.front().second);
}

