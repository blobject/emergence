#include "view.hh"

View::View(Processor* proc, bool visual, bool hidectrl)
{
  this->visual_ = visual;
  if (visual)
  {
    this->visualiser_ = new Visualiser(proc, hidectrl);
  }
  else
  {
    this->headless_ = new Headless(proc);
  }
}

void
View::Fin()
{
  if (this->visual_)
  {
    delete this->visualiser_;
  }
  else
  {
    delete this->headless_;
  }
}

