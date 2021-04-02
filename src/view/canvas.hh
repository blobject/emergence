//===-- canvas.hh - Canvas class declaration -------------------*- C++ -*-===//
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
#include "../util/log.hh"


class Gui;

class Canvas : public View, Observer
{
 public:
  /// constructor: Start observing Proc and Control, initialise Gui, and
  ///              prepare the OpenGL environment.
  /// \param log  Log object
  /// \param ctrl  Control object
  /// \param width  width of the graphical window
  /// \param height  height of the graphical height
  /// \param two  whether in 2D mode
  Canvas(Log& log, Control& ctrl, unsigned int width, unsigned int height,
         bool two);

  /// destructor: Detach from observation and delete the OpenGL constructs.
  ~Canvas() override;

  /// exec(): Render one iteration of all the graphics.
  ///         This is a virtual function of the View superclass.
  void exec() override;

  /// react(): React to State::change(), Proc::next(), Proc::done().
  ///          This is a virtual function of the Observer class.
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
  ///           Note that the 2D render still happens in a 3D environment.
  void next2d();

  /// next3d(): Update OpenGL vertex constructs for the 3D render.
  void next3d();

  /// shift(): Convenience function for next3d() that does one shift for a
  ///          given particle within a given vertex buffer (representing the
  ///          particle parameter), where "shifting" means drawing a copy of
  ///          all particles with an incremented z and alpha value, which
  ///          effectively shows passage of time.
  /// \param shift  whether shifting ought to happen.
  /// \param level  current number of levels
  /// \param next  new value of the particle parameter
  /// \param d  increment (delta) of the particle parameter
  /// \param v  vertex buffer being shifted
  /// \param i  particle index of the vertex buffer
  /// \param span  offset between levels in the vertex buffer, where the
  ///              buffer has been flattened to serially include all levels.
  void shift(bool shift, unsigned int level, GLfloat next, GLfloat d,
             std::vector<GLfloat>& v, unsigned int& i, unsigned int span);

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
  }

  /// pause(): Toggle pause of drawing and in Control.
  inline void
  pause()
  {
    this->paused_ = !this->paused_;
    this->ctrl_.pause(this->paused_);
  }

  /// quit: Turn off system processing.
  inline void
  quit()
  {
    this->ctrl_.quit();
  }

  /// camera_set(): Apply the current MVP matrices and set the corresponding
  ///               uniform for the vertex shader.
  inline void
  camera_set()
  {
    this->shader_->set_uniform_mat4f("mvp", this->proj_ * this->view_
                                     * this->model_ * this->orth_);
  }

  Gui*          gui_;
  Control&      ctrl_;
  VertexBuffer* vertex_buffer_xyz_;  // position vector buffer
  VertexBuffer* vertex_buffer_rgba_; // color vector buffer
  VertexBuffer* vertex_buffer_quad_; // quad (particle shape) vector buffer
  VertexArray*  vertex_array_;
  Shader*       shader_;
  GLfloat dollyd_;    // camera position delta
  GLfloat pivotd_;    // camera pivot angle delta
  GLfloat zoomd_;     // camera zoom delta
  bool paused_;       // no particle movement but accept user movement input

 private:
  Log& log_;
  std::vector<GLfloat> xyz_;  // position vector
  std::vector<GLfloat> rgba_; // color vector
  glm::mat4 orth_;            // orthogonalisation matrix
  glm::mat4 model_;           // model-to-world matrix
  glm::mat4 view_;            // world-to-view matrix
  glm::mat4 proj_;            // perspective-projection matrix
  glm::vec3 dolly_;           // camera position vector
  GLfloat width_;             // canvas width
  GLfloat height_;            // canvas width
  GLfloat pivotax_;           // camera horizontal pivot angle
  GLfloat pivotay_;           // camera vertical pivot angle
  GLfloat pivotx_;            // camera horizontal pivot amount
  GLfloat pivoty_;            // camera vertical pivot amount
  GLfloat zoomdef_;           // camera zoom default
  GLfloat neardef_;           // model's "near" default
  bool    three_;
  unsigned int levels_;       // total number of (z-)levels
  unsigned int level_;        // current number of levels
  unsigned int shift_counts_; // number of iterations until level shift
  unsigned int shift_count_;  // current iteration until level shift
  double ago_;                // previous time measurement
};

