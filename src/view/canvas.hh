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
  glm::mat4 orth_;
  glm::mat4 model_;
  glm::mat4 view_;
  glm::mat4 proj_;
  glm::vec3 dolly_;
  float zoomdef_;
  float panax_;
  float panay_;
  float panx_;
  float pany_;
  float dollyd_;
  float pand_;
  float zoomd_;
  double ago_;
  bool paused_;

 public:
  Sys*          sys_;
  Gui*          gui_;
  VertexBuffer* vertex_buffer_xyz_;
  VertexBuffer* vertex_buffer_quad_;
  VertexArray*  vertex_array_;
  Shader*       shader_;

  Canvas(Sys* sys, bool hide_ctrl);

  void Exec() override;
  void Spawn();
  void Respawn();
  void Clear();
  void Draw(unsigned int size, unsigned int count, VertexArray* va,
            //const IndexBuffer &ib,
            Shader* shader);
  void Next();
  void Pause();
  void CameraReset();
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

