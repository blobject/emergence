#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "visualiser.hh"
#include "../util/util.hh"


Visualiser::Visualiser(Sys* sys, bool hide_ctrl)
  : sys_(sys), shader_(NULL)
{
  State &state = sys->state_;
  auto gui_state = GuiState(state);
  this->gui_ = new Gui(gui_state, sys, "#version 330 core", state.width_,
                       state.height_);
  if (nullptr == this->gui_->window_)
  {
    return; // TODO: handle error
  }
  this->gui_->SetPointer();
	glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit())
  {
    Util::Err("glewInit");
  }
  this->ago_ = glfwGetTime();
  this->camera_ = { 0, 0, 0 };
  this->model_ = glm::translate(glm::mat4(1.0f), this->camera_);
  this->projection_ = glm::ortho(0.0f, static_cast<float>(state.width_),
                                 0.0f, static_cast<float>(state.height_),
                                -1.0f, 1.0f);
  this->view_ = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
}


void
Visualiser::Exec()
{
  this->gui_->SetVisualiser(this);
  Sys* sys = this->sys_;
  State &state = sys->state_;

  this->Spawn();

  // shading
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(state.width_),
                                    0.0f, static_cast<float>(state.height_),
                                   -1.0f, 1.0f);
  auto shader = new Shader();
  shader->Bind();
  shader->SetUniformMat4f("mvp", projection);
  this->shader_ = shader;

  /**
  // profiling
  GLuint64 start, stop;
  unsigned int queryID[2];
  glGenQueries(2, queryID);
  glQueryCounter(queryID[0], GL_TIMESTAMP);
  GLint stopAvailable = 0;
  bool profiled = false;
  //*/

  // timing
  double now;

  // render loop
  while (! this->gui_->Closing())
  {
    // timing
    now = glfwGetTime();
    if (now - this->ago_ < 1.0f / 60.0f)
    {
      continue;
    }
    this->ago_ = now;

    // pre
    this->Clear();

    // render
    //va.Bind(); // optional bind
    this->Draw(GL_FLOAT, state.num_, this->vertex_array_, shader);
    //va->Unbind(); // optional unbind
    this->gui_->Draw();

    // post
    sys->Next();
    this->gui_->Next();
    this->Next();

    /**
    // profiling
    glQueryCounter(queryID[1], GL_TIMESTAMP);
    while (! stopAvailable)
    {
      glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopAvailable);
    }
    if (! profiled)
    {
      profiled = true;
      glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
      glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
      std::cout << "time spent: " << (stop - start) / 1000000.0 << std::endl;
    }
    //*/
  }
}


// Spawn: Initialise OpenGL vertex constructs given particle positions and draw
//        them.

void
Visualiser::Spawn()
{
  State &state = this->sys_->state_;
  // initialise vertex buffers
  auto &particles = state.particles_;

  float size = particles[0].size / 2;
  float shape[] = { 0.0f, size,
                   -size, 0.0f,
                    size, 0.0f,
                    0.0f,-size };
  float trans[2 * state.num_];
  int index = 0;

  for (const Particle &particle : particles)
  {
    trans[index++] = particle.x;
    trans[index++] = particle.y;
  }

  // gpu buffers
  auto vbs = new VertexBuffer(shape, sizeof(shape));
  auto vbt = new VertexBuffer(trans, sizeof(trans));
  auto va = new VertexArray();
  va->AddBuffer(0, *vbs, VertexBufferLayout::Make<float>(2));
  va->AddBuffer(1, *vbt, VertexBufferLayout::Make<float>(2));

  // instancing
  glVertexAttribDivisor(1, 1);

  vbs->Unbind(); // optional
  vbt->Unbind(); // optional
  va->Unbind();
  this->vertex_buffer_shape_ = vbs;
  this->vertex_buffer_trans_ = vbt;
  this->vertex_array_ = va;
}


// Respawn: Reinitialise OpenGL vertex constructs and redraw particles.

void
Visualiser::Respawn()
{
  delete this->vertex_buffer_shape_;
  delete this->vertex_buffer_trans_;
  delete this->vertex_array_;
  this->Spawn();
}


void
Visualiser::Draw(unsigned int size, unsigned int count, VertexArray* va,
                 Shader* shader)
{
  shader->Bind();
  va->Bind();
  //ib->Bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, size, count));
  //DOGL(glDrawElements(GL_TRIANGLES, ib->get_count(), GL_UNSIGNED_INT, nullptr));
}


void
Visualiser::Clear()
{
  DOGL(glClear(GL_COLOR_BUFFER_BIT));
}


void
Visualiser::Next()
{
  State &state = this->sys_->state_;
  float trans[2 * state.num_];
  int index = 0;
  for (auto &particle : state.particles_)
  {
    trans[index++] = particle.x;
    trans[index++] = particle.y;
  }
  this->vertex_buffer_trans_->Update(trans);
  //this->vertex_array_->AddBuffer(1, *this->vertex_buffer_trans_,
  //                               VertexBufferLayout::Make<float>(2));
}


void
Visualiser::West()
{
  this->camera_ += glm::vec3(this->sys_->state_.width_ / 100.0f, 0, 0);
  this->model_ = glm::translate(glm::mat4(1.0f), this->camera_);
  this->shader_->Bind();
  this->shader_->SetUniformMat4f(
    "mvp", this->projection_ * this->view_ * this->model_);
}


void
Visualiser::South()
{
  this->camera_ += glm::vec3(0, this->sys_->state_.height_ / 100.0f, 0);
  this->model_ = glm::translate(glm::mat4(1.0f), this->camera_);
  this->shader_->Bind();
  this->shader_->SetUniformMat4f(
    "mvp", this->projection_ * this->view_ * this->model_);
}


void
Visualiser::North()
{
  this->camera_ += glm::vec3(0, this->sys_->state_.height_ / -100.0f, 0);
  this->model_ = glm::translate(glm::mat4(1.0f), this->camera_);
  this->shader_->Bind();
  this->shader_->SetUniformMat4f(
    "mvp", this->projection_ * this->view_ * this->model_);
}


void
Visualiser::East()
{
  this->camera_ += glm::vec3(this->sys_->state_.width_ / -100.0f, 0, 0);
  this->model_ = glm::translate(glm::mat4(1.0f), this->camera_);
  this->shader_->Bind();
  this->shader_->SetUniformMat4f(
    "mvp", this->projection_ * this->view_ * this->model_);
}


void
Visualiser::NorthWest()
{
  this->North();
  this->West();
}


void
Visualiser::NorthEast()
{
  this->North();
  this->East();
}


void
Visualiser::SouthWest()
{
  this->South();
  this->West();
}


void
Visualiser::SouthEast()
{
  this->South();
  this->East();
}

