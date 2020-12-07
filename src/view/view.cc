#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


std::unique_ptr<View>
View::Init(Log &log, State &state, Proc* proc, bool visual, bool hidectrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Canvas(log, state, proc, hidectrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(proc));
  return view;
}

