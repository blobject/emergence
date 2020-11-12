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
  double ago_;
  bool paused_;
  glm::vec3 camera_;
  glm::mat4 model_;
  glm::mat4 projection_;
  glm::mat4 view_;

 public:
  Sys*          sys_;
  Gui*          gui_;
  VertexBuffer* vertex_buffer_shape_;
  VertexBuffer* vertex_buffer_trans_;
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
  void West();
  void South();
  void North();
  void East();
  void NorthWest();
  void NorthEast();
  void SouthWest();
  void SouthEast();
};

