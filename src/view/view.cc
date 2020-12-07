#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


// Factory of view

std::unique_ptr<View>
View::Init(Log &log, State &state, Proc &proc, bool visual, bool hide_ctrl)
{
  if (visual)
  {
    std::unique_ptr<View> view(new Canvas(log, state, proc, hide_ctrl));
    return view;
  }
  std::unique_ptr<View> view(new Headless(log, proc));
  return view;
}

