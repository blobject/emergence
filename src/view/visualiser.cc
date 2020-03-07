#include <iostream>
#include <GL/glew.h>

#include "visualiser.hh"
#include "drawer.hh"

// tmp
#include <chrono>
#include <random>
#include "../state/state.hh"
// end tmp


Visualiser::Visualiser(Processor* processor, bool hide_ctrl)
  : processor_(processor), width_(800), height_(800)
{
  this->gui_ = new Gui("#version 330 core", this->width_, this->height_);
  if (nullptr == this->gui_->window_)
  {
    return;
  }
  if (GLEW_OK != glewInit())
  {
    std::cerr << "Error: glewInit" << std::endl;
  }
  // (NOTE: seems unnecessary) allow version 330 core shader syntax
  //putenv((char*) "MESA_GL_VERSION_OVERRIDE=3.3");
  //putenv((char*) "MESA_GLSL_VERSION_OVERRIDE=330");
}


Visualiser::~Visualiser()
{
  delete this->gui_;
}


void
Visualiser::Exec()
{
  GLFWwindow* window = this->gui_->window_;
  Drawer drawer;

  Shader shader("../etc/basic.glsl");
  shader.Bind();
  shader.SetUniform4f("uColor", 0.0f, 0.3f, 0.8f, 1.0f);

  float vertices[] = { -0.5f, -0.5f,
                        0.5f, -0.5f,
                        0.5f,  0.5f,
                       -0.5f,  0.5f };
  unsigned int indices[] = { 0, 1, 2,
                             2, 3, 0 };
  VertexArray va;
  VertexBuffer vb(vertices, 4 * 2 * sizeof(float));
  VertexBufferLayout layout;
  layout.Push<float>(2);
  va.AddBuffer(vb, layout);
  IndexBuffer ib(indices, 6);
  float red = 0.0f;
  float inc = 0.05f;
  va.Unbind();
  vb.Unbind();
  ib.Unbind();
  shader.Unbind();

  while (! this->gui_->Closing())
  {
    this->gui_->HandleInput();

    drawer.Clear();
    shader.Bind();
    shader.SetUniform4f("uColor", red, 0.3f, 0.8f, 1.0f);
    drawer.Draw(va, ib, shader);
    if (red > 1.0f) inc = -0.05f;
    else if (red < 0.0f) inc = 0.05f;
    red += inc;

    this->gui_->Draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

