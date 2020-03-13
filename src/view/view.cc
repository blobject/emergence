#include "view.hh"
#include "headless.hh"
#include "visualiser.hh"


std::unique_ptr<View>
View::Init(Processor* processor, bool visual, bool hidectrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Visualiser(processor, hidectrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(processor));
  return view;
}

