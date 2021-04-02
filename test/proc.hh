#include "../src/proc/proc.hh"


TEST_CASE("Control::Control")
{
  auto log = Log(1, true);
  auto state = State(log);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto exp = Exp(log, state);
  auto ctrl = Control(log, state, proc, exp, "");
  REQUIRE(-1 == ctrl.stop_);
}

