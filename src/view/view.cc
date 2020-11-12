#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


std::unique_ptr<View>
View::Init(Sys* sys, bool visual, bool hidectrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Canvas(sys, hidectrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(sys));
  return view;
}

