#include "view.hh"
#include "canvas.hh"
#include "headless.hh"


// Factory of Views

std::unique_ptr<View>
View::init(Log& log, Control& ctrl, bool headless, bool two)
{
  State& state = ctrl.get_state();
  if (headless) {
    std::unique_ptr<View> view(new Headless(log, ctrl));
    return view;
  }
  std::unique_ptr<View> view(new Canvas(log, ctrl,
                                        state.width_, state.height_,
                                        two));
  return view;
}

