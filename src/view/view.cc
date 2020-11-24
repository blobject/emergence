#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


std::unique_ptr<View>
View::Init(Proc* proc, bool visual, bool hidectrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Canvas(proc, hidectrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(proc));
  return view;
}

