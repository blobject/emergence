#include "proc.hh"
#include "../util/util.hh"


TEST_CASE("Proc::plot")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto grid = std::vector<int>();
  int cols;
  int rows;
  unsigned int gstride;

  proc.plot(state.scope_, grid, cols, rows, gstride);
  REQUIRE(static_cast<int>(state.width_ / state.scope_) == cols);
  REQUIRE(static_cast<int>(state.height_ / state.scope_) == rows);
  REQUIRE(0 < gstride);
  REQUIRE(cols * rows * gstride == grid.size());
}

