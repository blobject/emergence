#pragma once

// allow version 330 core shader syntax (NOTE: may be unnecessary)
#define MESA_GL_VERSION_OVERRIDE 3.3
#define MESA_GLSL_VERSION_OVERRIDE 330

#include <glm/glm.hpp>

#include "gui.hh"
#include "view.hh"
#include "../gl/vertexarray.hh"
#include "../gl/vertexbuffer.hh"
#include "../gl/shader.hh"
#include "../sys/sys.hh"


// Visualiser: The graphical view.

class Gui;

class Visualiser : public View
{
 private:
  double ago_;
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

  Visualiser(Sys* sys, bool hide_ctrl);

  void Exec() override;
  void Spawn();
  void Respawn();
  void Clear();
  void Draw(unsigned int size, unsigned int count, VertexArray* va,
            //const IndexBuffer &ib,
            Shader* shader);
  void Next();
  void West();
  void South();
  void North();
  void East();
  void NorthWest();
  void NorthEast();
  void SouthWest();
  void SouthEast();
};

