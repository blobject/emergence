#pragma once

#include <glm/glm.hpp>

#include "gl.hh"
#include "gui.hh"
#include "view.hh"
#include "../proc/proc.hh"
#include "../util/log.hh"
#include "../util/observation.hh"


// Canvas: The graphical view.

class Gui;

class Canvas : public View, Observer
{
 private:
  Log   &log_;
  State &state_;              // also the subject being observed
  std::vector<GLfloat> xyz_;  // position vector
  std::vector<GLfloat> rgba_; // color vector
  glm::mat4 orth_;            // orthogonalisation matrix
  glm::mat4 model_;           // model-to-world matrix
  glm::mat4 view_;            // world-to-view matrix
  glm::mat4 proj_;            // perspective projection matrix
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
  double ago_;

 public:
  Proc         &proc_;
  Gui*          gui_;
  VertexBuffer* vertex_buffer_xyz_;
  VertexBuffer* vertex_buffer_rgba_;
  VertexBuffer* vertex_buffer_quad_;
  VertexArray*  vertex_array_;
  Shader*       shader_;
  GLfloat dollyd_;   // camera position increment
  GLfloat pivotd_;   // camera pivot angle increment
  GLfloat zoomd_;    // camera zoom increment
  bool paused_;
  bool hard_paused_; // no input handling

  Canvas(Log &log, State &state, Proc &proc, bool hide_ctrl);
  ~Canvas() override;

  void Exec() override;
  void React(Subject&) override;
  void Spawn();
  void Respawn();
  void Clear();
  void Draw(GLuint instances, GLuint instance_count, VertexArray* va,
            Shader* shader);
  void Next2d();
  void Next3d();
  void Shift(bool shift, unsigned int level, GLfloat next, GLfloat inc,
             std::vector<GLfloat> &v, unsigned int &i, unsigned int span);
  void Three(bool yes);
  void HardPause(bool yes);
  void Pause();
  void CameraSet();
  void CameraDefault();
  void Camera(GLfloat dx, GLfloat dy, GLfloat dz, GLfloat dax, GLfloat day);
  void CameraResize(GLfloat w, GLfloat h);
};

