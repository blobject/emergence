#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


// Factory of Views

std::unique_ptr<View>
View::Init(Log &log, Control &ctrl, bool headless, bool hide_side)
{
  State &state = ctrl.GetState();
  if (headless)
  {
    std::unique_ptr<View> view(new Headless(log, ctrl));
    return view;
  }
  std::unique_ptr<View> view(new Canvas(log, ctrl,
                                        state.width_, state.height_,
                                        hide_side));
  return view;
}

