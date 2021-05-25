//===-- view/canvas.hh - Canvas class declaration --------------*- C++ -*-===//
///
/// \file
/// Declaration of the Canvas class, which is responsible for a graphical
/// depiction of the particles via OpenGL.
/// Canvas is a subclass of View and observes the Proc and Control classes.
/// Canvas uses the Gui class to provide user interaction.
///
//===---------------------------------------------------------------------===//

#pragma once

#include "gl.hh"
#include "gui.hh"
#include "view.hh"
#include "../util/common.hh"
#include "../util/log.hh"
#include <GLFW/glfw3.h>


class Gui;

class Canvas : public View, Observer
{
 public:
  /// constructor: Start observing Proc and Control, initialise Gui, and
  ///              prepare the OpenGL environment.
  /// \param log  Log object
  /// \param ctrl  Control object
  /// \param uistate  UiState object
  /// \param gui_on  whether GUI is enabled
  /// \param three  whether in 3D mode
  Canvas(Log& log, Control& ctrl, UiState& uistate, bool gui_on, bool three);

  /// preamble(): OpenGL related preamble.
  /// \param window_width  scaled window width
  /// \param window_height  scaled window height
  void preamble(unsigned int window_width, unsigned int window_height);

  /// destructor: Detach from observation and delete the OpenGL constructs.
  ~Canvas() override;

  /// intro(): (unimplemented)
  void intro() override;

  /// exec(): Render one iteration of all the graphics.
  void exec() override;

  /// react(): React to State::change(), Proc::next(), Proc::done().
  /// \param issue  which observed Subject's function to react to.
  void react(Issue issue) override;

  /// spawn(): Initialise OpenGL vertex constructs given particle positions.
  void spawn();

  /// respawn(): Reinitialise OpenGL vertex constructs.
  void respawn();

  /// draw(): Execute the OpenGL draw call.
  /// \param instances  number of indices to be rendered per particle.
  /// \param instance_count  number of particles to be rendered.
  /// \param vertex_array  OpenGL vertex array containing particle data.
  /// \param shader  OpenGL shader object.
  void draw(GLuint instances, GLuint instance_count,
            VertexArray* vertex_array, Shader* shader) const;

  /// next2d(): Update OpenGL vertex constructs for the 2D render.
  ///           Note: 2D rendering still happens in a 3D environment.
  void next2d();

  /// next3d(): Update OpenGL vertex constructs for the 3D render.
  void next3d();

  /// camera_default(): Apply the default MVP matrices.
  void camera_default();

  /// camera(): Change the MVP matrices.
  void camera(GLfloat dx, GLfloat dy, GLfloat dz, GLfloat dax, GLfloat day);

  /// camera_resize(): Change the projection matrix to account for new
  ///                  window dimensions.
  void camera_resize(GLfloat w, GLfloat h);

  /// clear(): Clear the OpenGL buffers (between every iteration).
  inline void
  clear() const
  {
    DOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  }

  /// three(): Set 3D mode.
  /// \param yesno  whether mode ought to be 3D.
  inline void
  three(bool yesno)
  {
    this->three_ = yesno;
    this->level_ = 1;
    this->shift_count_ = 0;
  }

  /// trail(): Set trailing.
  /// \param yesno  whether trailing ought to be enabled.
  inline void
  trail(bool yesno)
  {
    this->trail_ = yesno;
    this->trail_count_ = 0;
    this->trail_end_ = false;
  }

  /// camera_set(): Apply the current MVP matrices and set the corresponding
  ///               uniform for the vertex shader.
  inline void
  camera_set()
  {
    this->shader_->set_uniform_mat4f("mvp", this->proj_ * this->view_
                                     * this->model_ * this->orth_);
  }

  /// next(): Swap OpenGL buffers and poll for events.
  void next() const;

  /// set_pointer(): Create a pointer to self for access in callbacks.
  void set_pointer();

  /// close(): Begin shutting down window.
  void close();

  /// closing(): Whether window is shutting down.
  bool closing() const;

  /// key_callback_no_gui(): User key input bindings (only pause, step, quit).
  /// \param window  pointer to the window object
  /// \param key  engaged key
  /// \param scancode  key scancode
  /// \param action  manner of key engagement
  /// \param mods  engaged modifier keys
  static void key_callback_no_gui(GLFWwindow* window, int key, int scancode,
                                  int action, int mods);

  // resize_callback: Window resize bindings.
  /// \param window  pointer to the window object
  /// \param w  new window width
  /// \param h  new window height
  static void resize_callback(GLFWwindow* window, int w, int h);

  GLFWwindow*   window_;
  Gui*          gui_;
  Control&      ctrl_;
  VertexBuffer* vertex_buffer_xyz_;  // position vector buffer
  VertexBuffer* vertex_buffer_rgba_; // color vector buffer
  VertexBuffer* vertex_buffer_quad_; // quad (particle shape) vector buffer
  VertexArray*  vertex_array_;
  Shader*       shader_;
  GLfloat dollyd_; // camera position delta
  GLfloat pivotd_; // camera pivot angle delta
  GLfloat zoomd_;  // camera zoom delta

 private:
  Log&      log_;
  std::vector<GLfloat> xyz_;  // position vector
  std::vector<GLfloat> rgba_; // color vector
  GLfloat   width_;           // canvas width
  GLfloat   height_;          // canvas width
  bool      gui_on_;          // whether GUI is enabled
  bool      three_;           // whether in 3D mode
  bool      trail_;           // whether trailing is enabled
  unsigned int trail_count_;  // current trail iteration
  bool      trail_end_;       // whether maximum trailing has been reached
  unsigned int levels_;       // total number of (z-)levels
  unsigned int level_;        // current number of levels
  unsigned int shift_counts_; // number of iterations until level shift
  unsigned int shift_count_;  // current iteration until level shift
  glm::mat4 orth_;            // orthogonalisation matrix
  glm::mat4 model_;           // model-to-world matrix
  glm::mat4 view_;            // world-to-view matrix
  glm::mat4 proj_;            // perspective-projection matrix
  glm::vec3 dolly_;           // camera position vector
  GLfloat   pivotax_;         // camera horizontal pivot angle
  GLfloat   pivotay_;         // camera vertical pivot angle
  GLfloat   pivotx_;          // camera horizontal pivot amount
  GLfloat   pivoty_;          // camera vertical pivot amount
  GLfloat   zoomdef_;         // camera zoom default
  GLfloat   neardef_;         // model's "near" default
  double    ago_;             // previous time measurement
};

