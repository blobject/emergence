#pragma once

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "canvas.hh"
#include "../proc/control.hh"
#include "../util/log.hh"
#include "../util/util.hh"


// GuiState: Intermediary state storage before really modifying State

class GuiState
{
 private:
  Control &ctrl_;

 public:
  long long    stop_;
  int          num_;
  unsigned int width_;
  unsigned int height_;
  float        alpha_;
  float        beta_;
  float        scope_;
  float        speed_;
  int          colors_;
  int          preset_;

  GuiState(Control &ctrl);

  bool Untrue();
  bool ChangeTruth();
  void Random(Log &log);
  void Preset(Log &log);
  bool Save(const std::string &path);
  bool Load(const std::string &path);
};


// Gui: The part of the visualisation dealing with the user interface.

class Canvas;

class Gui
{
 private:
  GuiState     state_;
  Log         &log_;
  ImFont*      font_r;
  ImFont*      font_b;
  ImFont*      font_i;
  ImFont*      font_z;
  int          font_width_;
  unsigned int gui_width_;
  unsigned int gui_height_;
  bool         side_;
  bool         console_;
  char         dialog_;
  double       ago_;
  unsigned int frames_;
  float        fps_;
  double       x_;
  double       y_;
  bool         dolly_;
  bool         pivot_;

 public:
  Canvas&     canvas_;
  GLFWwindow* view_;

  Gui(Log &log, GuiState state, Canvas &canvas,
      unsigned int width, unsigned int height, bool hide_side);
  ~Gui();

  void Draw();
  void DrawSide(bool draw);
  void DrawConsole(bool draw);
  void DrawSaveLoad(char dialog);
  void DrawQuit(char dialog);
  void Next() const;
  void Pause();
  void SetPointer();
  void Close();
  bool Closing() const;
  static void KeyCallback(GLFWwindow* view, int key, int scancode, int action,
                          int mods);
  static void MouseButtonCallback(GLFWwindow* view, int button, int action,
                                  int mods);
  static void MouseMoveCallback(GLFWwindow* view, double x, double y);
  static void MouseScrollCallback(GLFWwindow* view, double dx, double dy);
  static void ResizeCallback(GLFWwindow* view, int w, int h);
};

