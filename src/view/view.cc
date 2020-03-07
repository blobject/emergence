#include "view.hh"
#include "headless.hh"
#include "visualiser.hh"


View*
View::Init(Processor* processor, bool visual, bool hidectrl)
{
  if (visual)
  {
    return new Visualiser(processor, hidectrl);
  }
  return new Headless(processor);
}

