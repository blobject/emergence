#pragma once

#include <glm/glm.hpp>

#include "gl.hh"
#include "gui.hh"
#include "view.hh"
#include "../proc/proc.hh"


// Canvas: The graphical view.

class Gui;

class Canvas : public View
{
 private:
  std::vector<GLfloat> xyz_;  // position vector
  std::vector<GLfloat> rgba_; // color vector
  glm::mat4 orth_;            // orthogonalisation matrix
  glm::mat4 model_;           // model-to-world matrix
  glm::mat4 view_;            // world-to-view matrix
  glm::mat4 proj_;            // perspective projection matrix
  glm::vec3 dolly_;           // camera position vector
  GLfloat panax_;             // camera horizontal pan angle
  GLfloat panay_;             // camera vertical pan angle
  GLfloat panx_;              // camera horizontal pan amount
  GLfloat pany_;              // camera vertical pan amount
  GLfloat dollyd_;            // camera position increment
  GLfloat pand_;              // camera pan angle increment
  GLfloat zoomd_;             // camera zoom increment
  GLfloat zoomdef_;           // camera zoom default
  GLfloat neardef_;           // model's "near" default
  unsigned int levels_;             // total number of (z-)levels
  unsigned int level_;              // current number of levels
  unsigned int shift_counts_; // number of iterations until level shift
  unsigned int shift_count_;  // current iteration until level shift
  double ago_;
  bool paused_;

 public:
  Proc*         proc_;
  Gui*          gui_;
  VertexBuffer* vertex_buffer_xyz_;
  VertexBuffer* vertex_buffer_rgba_;
  VertexBuffer* vertex_buffer_quad_;
  VertexArray*  vertex_array_;
  Shader*       shader_;

  Canvas(Proc* proc, bool hide_ctrl);

  void Exec() override;
  void Spawn();
  void Respawn();
  void Clear();
  void Draw(GLuint instances, GLuint instance_count, VertexArray* va,
            Shader* shader);
  void Next();
  void Shift(bool shift, unsigned int level, float n, float d,
             std::vector<GLfloat> &v, unsigned int &i, unsigned int span);
  void Pause();
  void CameraSet();
  void CameraDefault();
  void DollyWest();
  void DollySouth();
  void DollyNorth();
  void DollyEast();
  void DollyNorthWest();
  void DollyNorthEast();
  void DollySouthWest();
  void DollySouthEast();
  void PanWest();
  void PanSouth();
  void PanNorth();
  void PanEast();
  void PanNorthWest();
  void PanNorthEast();
  void PanSouthWest();
  void PanSouthEast();
  void PushIn();
  void PullOut();
};

