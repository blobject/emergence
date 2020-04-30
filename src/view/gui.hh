#pragma once

#include <GLFW/glfw3.h>

#include "visualiser.hh"
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
  int          colorscheme_;
  unsigned int num_;
  unsigned int width_;
  unsigned int height_;
  float        scope_;
  float        speed_;
  float        alpha_;
  float        beta_;

  GuiState(State &truth);

  void ChangeTruth();
  bool Untrue();
};


// Gui: The part of the visualisation dealing with the user interface.

class Visualiser;

class Gui
{
 private:
  GuiState     state_;
  double       ago_;
  unsigned int frames_;
  float        fps_;

  bool IsItemJustReleased();

 public:
  Sys*        sys_;
  Visualiser* visualiser_;
  GLFWwindow* window_;

  Gui(GuiState state, Sys* sys, const std::string &version,
      unsigned int width, unsigned int height);
  ~Gui();

  inline GuiState get_state() const { return this->state_; }
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mods);
  void SetPointer();
  void SetVisualiser(Visualiser* visualiser);
  void Close();
  bool Closing() const;
  void Draw();
  void Next() const;
};

