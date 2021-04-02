//===-- gui.hh - Gui class definition --------------------------*- C++ -*-===//
///
/// \file
/// Definition of the GuiState struct and declaration the Control class, which
/// is responsible for graphically rendering (via OpenGL) and managing the main
/// window and the user interface elements.
/// The GuiState struct is an intermediary State parameter storage before truly
/// modifying State.
/// Gui is owned and called by the Canvas class (Canvas tells Gui to be drawn,
/// although GLFW (a member object of Gui) is ultimately the one overseeing the
/// entire graphical context).
/// Gui communicates with the Control class in order to access/modify State and
/// handle Proc.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "canvas.hh"
#include "../proc/control.hh"
#include "../util/log.hh"
#include "../util/util.hh"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


class GuiState
{
public:
  /// constructor: Initialise GuiState with Control-provided State
  ///              parameters.
  /// \param ctrl  Control object
  GuiState(Control& ctrl);

  /// current(): Get current Gui's state.
  /// \returns  snapshot of current Gui's state
  Stative current() const;

  /// untrue(): Ask Control whether Gui's parameters are different from
  ///           State's.
  /// \returns  true if Gui's state is different from true State
  bool untrue() const;

  /// deceive(): Change true State parameters.
  /// \returns  true if particle parameters were respawned
  bool deceive() const;

  /// random(): Randomise true State parameters.
  /// \param log  Log object
  void random(Log& log);

  /// coloring(): Apply coloring.
  /// \param log  Log object
  /// \param scheme  particle coloring scheme
  void coloring(Log& log, Coloring scheme);

  /// preset(): Apply preset parameters (ALPHA & BETA) to true State.
  /// \param log  Log object
  void preset(Log& log);

  /// save(): Thin wrapper around Control.Save().
  /// \param path  string of path to the save file
  bool save(const std::string& path);

  /// load(): Thin wrapper around Control.load().
  ///         Also update GuiState parameters immediately as a difference
  ///         check would be circuitous.
  /// \param path  string of path to the load file
  bool load(const std::string& path);

  Control&     ctrl_;
  long long    stop_;
  int          num_;
  unsigned int width_;
  unsigned int height_;
  float        alpha_;
  float        beta_;
  float        scope_;
  float        speed_;
  float        prad_;
  Coloring     coloring_;
  int          preset_;
};


class Canvas;

class Gui
{
 public:
  /// constructor: Initialise the window and the UI, that is, those graphical
  ///              entities that are not specifically related to the
  ///              particles.
  /// \param log  Log object
  /// \param state  GuiState object
  /// \param canvas  Canvas object
  /// \param width  initial window width
  /// \param height  initial window height
  /// \param two  whether in 2D mode
  Gui(Log& log, GuiState state, Canvas& canvas,
      unsigned int width, unsigned int height, bool two);

  /// destructor: Clean up the window and the UI.
  ~Gui();

  /// draw(): Render the window and the UI.
  void draw();

  /// draw_brief(): Render the brief status information.
  /// \param draw  whether the brief information should be drawn
  void draw_brief(bool draw);

  /// draw_messages(): Render the message log.
  /// \param draw  whether the message log area should be drawn
  void draw_messages(bool draw);

  /// draw_config(): Render the configuration dialog.
  /// \param draw  whether the configuration dialog should be drawn
  void draw_config(char dialog);

  /// draw_save_load(): Render the save/load dialog.
  /// \param dialog  whether the 's'ave or the 'l'oad dialog should be drawn
  void draw_save_load(char dialog);

  /// draw_quit(): Render the quit confirmation dialog.
  /// \param dialog  whether the 'q'uit dialog should be drawn
  void draw_quit(char dialog);

  /// backspace(): Move cursor back one character.
  /// \param offset  how many more pixels to add
  inline void
  backspace(int offset = 1)
  {
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - this->font_width_ - offset);
  }

  /// auto_width(): Set next element width.
  /// \param factor  how much to divide the remaining space by
  void auto_width(int factor);

  /// next(): Swap OpenGL buffers and poll for events.
  void next() const;

  /// pause(): Toggle pause in Canvas.
  void pause();

  /// set_pointer(): Create a pointer to self for access in callbacks.
  void set_pointer();

  /// close(): Begin shutting down the window.
  void close();

  /// closing(): Whether the window is shutting down.
  bool closing() const;

  /// key_callback(): User key input bindings.
  /// \param view  pointer to the window object
  /// \param key  engaged key
  /// \param scancode  key scancode
  /// \param action  manner of key engagement
  /// \param mods  engaged modifier keys
  static void key_callback(GLFWwindow* view, int key, int scancode,
                           int action, int mods);

  /// mouse_move_callback: User mouse movement bindings.
  /// \param view  pointer to the window object
  /// \param x  mouse cursor's x position
  /// \param y  mouse cursor's y position
  static void mouse_move_callback(GLFWwindow* view, double x, double y);

  /// mouse_button_callback(): User mouse input bindings.
  /// \param view  pointer to the window object
  /// \param key  engaged button
  /// \param action  manner of key engagement
  /// \param mods  engaged modifier keys
  static void mouse_button_callback(GLFWwindow* view, int button, int action,
                                    int mods);

  /// mouse_scroll_callback: User mouse scrolling bindings.
  /// \param view  pointer to the window object
  /// \param dx  amount of horizontal mouse scroll
  /// \param dy  amount of vertical mouse scroll
  static void mouse_scroll_callback(GLFWwindow* view, double dx, double dy);

  // resize_callback: Window resize bindings.
  /// \param view  pointer to the window object
  /// \param w  new window width
  /// \param h  new window height
  static void resize_callback(GLFWwindow* view, int w, int h);

  Canvas&     canvas_;
  GLFWwindow* view_;

 private:
  GuiState     state_;
  Log&         log_;
  float        dpi_;
  ImFont*      font_r;
  ImFont*      font_b;
  ImFont*      font_i;
  ImFont*      font_z;
  int          font_width_;
  unsigned int gui_width_;
  unsigned int gui_height_;
  bool         brief_;       // whether the brief information should be shown
  bool         messages_;    // whether the message log area should be shown
  char         dialog_;      // which modal dialog should be shown
  double       ago_;         // last moment when counting of frames began (~1s)
  unsigned int frames_;      // accumulated number of draws
  float        fps_;         // calculated frames per second
  double       x_;           // mouse cursor's x position
  double       y_;           // mouse cursor's y position
  bool         three_;       // whether in 3D mode
  bool         dolly_;       // whether mouse activated camera's dolly movement
  bool         pivot_;       // whether mouse activated camera's pivot movement
  float        cluster_radius_; // radius for dbscan
  unsigned int cluster_minpts_; // minpts for dbscan
  int          density_threshold_; // current density mapping threshold
  std::string  message_exp_cluster_; // cluster-analysis-related message
  std::string  message_exp_inject_;  // injection-analysis-related message
  std::string  message_exp_density_; // density-analysis-related message
};

