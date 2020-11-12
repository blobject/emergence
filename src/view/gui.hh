#pragma once

#include <GLFW/glfw3.h>

#include "canvas.hh"
#include "../sys/sys.hh"
#include "../util/util.hh"


// GuiState: Intermediary state holding before modifying State

class GuiState
{
 private:
  State &truth_;

 public:
  // stable
  Distribution distribution_;
  unsigned int stop_;
  // adjustable
  unsigned int num_;
  unsigned int width_;
  unsigned int height_;
  float        alpha_;
  float        beta_;
  float        scope_;
  float        speed_;
  int          colorscheme_;

  GuiState(State &truth);

  bool ChangeTruth();
  bool Untrue();
};


// Gui: The part of the visualisation dealing with the user interface.

class Canvas;

class Gui
{
 private:
  GuiState     state_;
  double       ago_;
  unsigned int frames_;
  float        fps_;
  bool         control_;
  unsigned int control_width_;

  bool IsItemJustReleased();

 public:
  Sys*        sys_;
  Canvas*     canvas_;
  GLFWwindow* view_;

  Gui(GuiState state, Sys* sys, const std::string &version,
      unsigned int width, unsigned int height);
  ~Gui();

  inline GuiState get_state() const { return this->state_; }
  static void KeyCallback(GLFWwindow* view, int key, int scancode, int action,
                          int mods);
  static void SizeCallback(GLFWwindow* view, int w, int h);
  void SetPointer();
  void SetCanvas(Canvas* canvas);
  void Close();
  bool Closing() const;
  void Draw();
  void DrawControl(bool draw);
  void Next() const;
};

