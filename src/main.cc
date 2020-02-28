#include <cstdlib>
#include "const.hh"
#include "controller/controller.hh"
#include "model/model.hh"
#include "view/view.hh"

int main()
{
  Model model;
  Controller controller;
  View view;

  view.Draw(view.window);
  view.End();

  return 0;
}
