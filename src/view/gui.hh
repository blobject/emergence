//===-- view/gui.hh - Gui class definition ---------------------*- C++ -*-===//
///
/// \file
/// Definition of the Box enum and declaration of the Gui class, which is
/// responsible for graphically rendering and managing the graphical user
/// interface elements.
/// Gui is owned and called by the Canvas class (Canvas tells Gui to be drawn).
/// Gui communicates with the Control class in order to access/modify State and
/// handle Proc.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "canvas.hh"
#include "image.hh"
#include "state.hh"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


// Box: Type of dialog box.

enum class Box
{
  None = 0,
  Config,
  Capture,
  Save,
  Load,
  Quit
};


class Canvas;

class Gui
{
 public:
  /// constructor: Initialise the window and the UI, that is, those graphical
  ///              entities that are not depended by the particle system.
  /// \param log  Log object
  /// \param uistate  UiState object
  /// \param canvas  Canvas object
  /// \param window  GLFW window object
  /// \param scale  window scaling
  /// \param three  whether in 3D mode
  Gui(Log& log, UiState& uistate, Canvas& canvas, GLFWwindow* window,
      float scale, bool three);

  /// destructor: Clean up the window and the UI.
  ~Gui();

  /// draw(): Render the window and the UI.
  void draw();

  int capturing_; // workaround to close render boxes during capture

 private:
  /// draw_brief(): Render the brief status information.
  /// \param draw  whether the brief information should be drawn
  void draw_brief(bool draw);

  /// draw_messages(): Render the message log.
  /// \param draw  whether the message log area should be drawn
  void draw_messages(bool draw);

  /// draw_config(): Render the configuration box.
  /// \param draw  whether the onfiguration box should be drawn
  void draw_config(Box box);

  /// draw_config_top(): Render configuration box in parts.
  /// \param width  (some) width of configuration box
  void draw_config_top();
  void draw_config_graphics(float width);
  void draw_config_habitat(float width);
  void draw_config_analysis(float width);
  void draw_config_usage();

  /// draw_capture(): Render the capture box.
  /// \param box  whether the ca'p'ture box should be drawn
  void draw_capture(Box box);

  /// draw_load_save(): Render the load/save box.
  /// \param box  whether the 's'ave or the 'l'oad box should be drawn
  void draw_load_save(Box box);

  /// draw_quit(): Render the quit confirmation box.
  /// \param box  whether the 'q'uit box should be drawn
  void draw_quit(Box box);

  /// box_confirm(): Perform the main box action.
  ///                Used when rendering boxes and when pressing Enter.
  /// \param box  which box the confirmation applies to
  void box_confirm(Box box);

  /// gen_message_exp_inspect(): Compute inspection message.
  void gen_message_exp_inspect();

  /// backspace(): Move cursor back one space.
  /// \param offset  how many more/less pixels to move back
  inline void
  backspace(int offset = 1)
  {
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - this->font_width_ - offset);
  }

  /// auto_width(): Set next element width.
  /// \param width  width of box box
  /// \param factor  how much to divide the remaining space by
  void auto_width(int width, float factor = 1.0f);

  /// key_callback(): User key input bindings.
  /// \param window  pointer to the window object
  /// \param key  engaged key
  /// \param scancode  key scancode
  /// \param action  manner of key engagement
  /// \param mods  engaged modifier keys
  static void key_callback(GLFWwindow* window, int key, int scancode,
                           int action, int mods);

  /// mouse_move_callback: User mouse movement bindings.
  /// \param window  pointer to the window object
  /// \param x  mouse cursor's x position
  /// \param y  mouse cursor's y position
  static void mouse_move_callback(GLFWwindow* window, double x, double y);

  /// mouse_button_callback(): User mouse input bindings.
  /// \param window  pointer to the window object
  /// \param key  engaged button
  /// \param action  manner of key engagement
  /// \param mods  engaged modifier keys
  static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                    int mods);

  /// mouse_scroll_callback: User mouse scrolling bindings.
  /// \param window  pointer to the window object
  /// \param dx  amount of horizontal mouse scroll
  /// \param dy  amount of vertical mouse scroll
  static void mouse_scroll_callback(GLFWwindow* window, double dx, double dy);

  // resize_callback: Window resize bindings.
  /// \param window  pointer to the window object
  /// \param w  new window width
  /// \param h  new window height
  static void resize_callback(GLFWwindow* window, int w, int h);

  Log&         log_;
  UiState&     uistate_;
  GLFWwindow*  window_;
  Canvas&      canvas_;
  float        scale_;
  double       ago_;         // last moment when counting of frames began (~1s)
  unsigned int frames_;      // accumulated number of draws
  float        fps_;         // calculated frames per second
  double       x_;           // mouse cursor's x position
  double       y_;           // mouse cursor's y position
  bool         trail_;       // whether trailing is enabled
  bool         three_;       // whether in 3D mode
  bool         dolly_;       // whether mouse activated camera's dolly movement
  bool         pivot_;       // whether mouse activated camera's pivot movement
  std::string  cwd_;
  ImFont*      font_r_;
  ImFont*      font_b_;
  ImFont*      font_i_;
  ImFont*      font_z_;
  int          font_width_;
  ImVec4       text_color_good_;
  ImVec4       text_color_bad_;
  ImVec4       text_color_dim_;
  ImVec4       text_color_dimmer_;
  bool         brief_;       // whether brief information should be shown
  bool         messages_;    // whether message log area should be shown
  Box          box_;         // which dialog box should be shown
  float        box_opacity_; // opacity of dialog boxes
  bool         input_focus_; // focus marker for when the dialog box opens
  std::string  input_allowed_;       // regex string of allowed input chars
  char         capture_path_[256];   // path to taken picture
  char         load_path_[256];      // path to load
  char         save_path_[256];      // path to save
  char         quit_save_path_[256]; // path to quit save
  int          capture_;        // picture taking status (0=none,1=good,0>bad)
  int          load_save_;      // load/save status (0=none,1=good,0>bad)
  int          quit_;           // quit status (0=none,1=good,0>bad)
  int          coloring_;       // particle coloring scheme
  float        cluster_radius_; // DBSCAN radius
  unsigned int cluster_minpts_; // DBSCAN minpts
  int          inject_sprite_;  // particle cluster sprite model to be injected
  float        inject_dpe_;     // DPE after injection
  bool         inspect_greater_; // whether clusters include greater nbhds
  int          inspect_particle_;         // particle index under inspection
  int          inspect_cluster_;          // cluster index under inspection
  int          inspect_cluster_particle_; // cluster particle index under insp
  std::string  message_set_;         // habitat-preset-related message
  std::string  message_exp_color_;   // coloring-related message
  std::string  message_exp_cluster_; // clustering-related message
  std::string  message_exp_inject_;  // injection-related message
  std::string  message_exp_inspect_; // inspection-related message
  std::string  message_exp_inspect_default_;
};

