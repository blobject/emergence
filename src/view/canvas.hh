#pragma once

#include <glm/glm.hpp>

#include "gl.hh"
#include "gui.hh"
#include "view.hh"
#include "../sys/sys.hh"


// Canvas: The graphical view.

class Gui;

class Canvas : public View
{
 private:
  std::vector<GLfloat> xyz_;
  std::vector<GLfloat> rgba_;
  glm::mat4 orth_;
  glm::mat4 model_;
  glm::mat4 view_;
  glm::mat4 proj_;
  glm::vec3 dolly_;
  GLfloat panax_;
  GLfloat panay_;
  GLfloat panx_;
  GLfloat pany_;
  GLfloat dollyd_;
  GLfloat pand_;
  GLfloat zoomd_;
  GLfloat zoomdef_;
  GLfloat neardef_;
  unsigned int levels_;
  unsigned int level_;
  unsigned int level_shift_counts_;
  unsigned int level_shift_count_;
  double ago_;
  bool paused_;

 public:
  Sys*          sys_;
  Gui*          gui_;
  VertexBuffer* vertex_buffer_xyz_;
  VertexBuffer* vertex_buffer_rgba_;
  VertexBuffer* vertex_buffer_quad_;
  VertexArray*  vertex_array_;
  Shader*       shader_;

  Canvas(Sys* sys, bool hide_ctrl);

  void Exec() override;
  void Spawn();
  void Respawn();
  void Clear();
  void Draw(GLuint instances, GLuint instance_count, VertexArray* va,
            Shader* shader);
  void Next();
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

