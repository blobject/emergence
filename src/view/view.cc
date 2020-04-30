#include "view.hh"
#include "headless.hh"
#include "visualiser.hh"


std::unique_ptr<View>
View::Init(Sys* sys, bool visual, bool hidectrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Visualiser(sys, hidectrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(sys));
  return view;
}

