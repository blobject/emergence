#include "proc.hh"


TEST_CASE("Control::Control")
{
  auto log = Log(1, true);
  auto state = State(log, 0);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto exp = Exp(log, state, proc, false);
  auto ctrl = Control(log, state, proc, exp, "", false);
  REQUIRE(-1 == ctrl.countdown_);
}

