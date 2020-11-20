#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Sys* sys, bool hide_ctrl)
  : sys_(sys), shader_(NULL)
{
  State &state = sys->state_;
  auto gui_state = GuiState(state);
  this->gui_ = new Gui(gui_state, sys, "#version 330 core", state.width_,
                       state.height_);
  this->gui_->SetCanvas(this);
  if (nullptr == this->gui_->view_)
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
  this->paused_ = false;
}


void
Canvas::Exec()
{
  DOGL(glEnable(GL_DEPTH_TEST));

  Sys* sys = this->sys_;
  State &state = sys->state_;

  auto w = state.width_;
  auto h = state.height_;

  float     zoomdef = -3.2f;
  glm::vec3 dolly = glm::vec3(0.0f, 0.0f, zoomdef);
  float     panax = 0.0f;
  float     panay = 0.0f;
  float     panx = 1.0f;
  float     pany = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, dolly);
  view = glm::rotate(view, glm::radians(panax), glm::vec3(panx, pany, 1.0f));
  glm::mat4 orth = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -50.0f, 50.0f);
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 proj = glm::perspective(glm::radians(50.0f),
                          (float)w / (float)h, 0.1f, 100.0f);
  this->dolly_ = dolly;
  this->panax_ = panax;
  this->panay_ = panay;
  this->panx_ = panx;
  this->pany_ = pany;
  this->view_ = view;
  this->orth_ = orth;
  this->model_ = model;
  this->proj_ = proj;

  this->zoomdef_ = zoomdef;
  this->dollyd_ = 0.05f;
  this->pand_ = 0.5f;
  this->zoomd_ = 0.05f;

  this->Spawn();

  /**
  // profiling
  GLuint64 start, stop;
  unsigned int queryID[2];
  glGenQueries(2, queryID);
  glQueryCounter(queryID[0], GL_TIMESTAMP);
  GLint stopAvailable = 0;
  bool profiled = false;
  //*/

  auto va = this->vertex_array_;
  auto shader = new Shader();
  shader->Bind();
  shader->SetUniformMat4f("mvp", proj * view * model * orth);
  this->shader_ = shader;

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

    if (!this->paused_)
    {
      this->Clear();
      this->Draw(GL_FLOAT, state.num_, va, shader);
      sys->Next();
      this->Next();
    }

    this->gui_->Draw();
    this->gui_->Next();

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
Canvas::Spawn()
{
  State &state = this->sys_->state_;
  // initialise vertex buffers
  auto &particles = state.particles_;

  GLfloat prad = particles[0].rad;

  GLfloat xyz[3 * state.num_];
  int pindex = 0;
  for (const Particle &particle : particles)
  {
    xyz[pindex++] = particle.x;
    xyz[pindex++] = particle.y;
    xyz[pindex++] = -50.0f;
  }

  GLfloat quad[] = { 0.0f, prad,
                    -prad, 0.0f,
                     prad, 0.0f,
                     0.0f,-prad };

  // gpu buffers
  auto vbx = new VertexBuffer(xyz, sizeof(xyz));
  auto vbq = new VertexBuffer(quad, sizeof(quad));
  auto va = new VertexArray();
  va->AddBuffer(0, *vbx, VertexBufferLayout::Make<float>(3));
  va->AddBuffer(1, *vbq, VertexBufferLayout::Make<float>(2));

  // instancing
  glVertexAttribDivisor(0, 1);

  vbx->Unbind();
  vbq->Unbind();
  va->Unbind();
  this->vertex_buffer_xyz_ = vbx;
  this->vertex_buffer_quad_ = vbq;
  this->vertex_array_ = va;
}


// Respawn: Reinitialise OpenGL vertex constructs and redraw particles.

void
Canvas::Respawn()
{
  delete this->vertex_buffer_xyz_;
  delete this->vertex_buffer_quad_;
  delete this->vertex_array_;
  this->Spawn();
}


void
Canvas::Draw(unsigned int size, unsigned int count, VertexArray* va,
             Shader* shader)
{
  shader->Bind();
  va->Bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, size, count));
}


void
Canvas::Clear()
{
  DOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}


void
Canvas::Next()
{
  State &state = this->sys_->state_;
  GLfloat xyz[3 * state.num_];
  int pindex = 0;
  for (auto &particle : state.particles_)
  {
    xyz[pindex++] = particle.x;
    xyz[pindex++] = particle.y;
    xyz[pindex++] = -50.0f;
  }
  this->vertex_buffer_xyz_->Update(xyz);
}


void
Canvas::Pause()
{
  this->paused_ = !this->paused_;
}


void
Canvas::CameraReset()
{
  unsigned int w = this->sys_->state_.width_;
  unsigned int h = this->sys_->state_.height_;
  this->dolly_ = glm::vec3(0.0f, 0.0f, this->zoomdef_);
  this->panax_ = 0.0f;
  this->panay_ = 0.0f;
  this->panx_ = 1.0f;
  this->pany_ = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, this->dolly_);
  this->orth_ = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -50.0f, 50.0f);
  this->model_ = glm::mat4(1.0f);
  this->view_ = glm::rotate(view, glm::radians(this->panax_),
                            glm::vec3(this->panx_, this->pany_, 1.0f));
  this->proj_ = glm::perspective(glm::radians(50.0f),
                                 (float)w / (float)h, 0.1f, 100.0f);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::DollyNorth()
{
  this->dolly_.y -= this->dollyd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::DollyWest()
{
  this->dolly_.x += this->dollyd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::DollySouth()
{
  this->dolly_.y += this->dollyd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::DollyEast()
{
  this->dolly_.x -= this->dollyd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::DollyNorthWest() { this->DollyNorth(); this->DollyWest(); }

void
Canvas::DollyNorthEast() { this->DollyNorth(); this->DollyEast(); }

void
Canvas::DollySouthWest() { this->DollySouth(); this->DollyWest(); }

void
Canvas::DollySouthEast() { this->DollySouth(); this->DollyEast(); }


void
Canvas::PanNorth()
{
  this->panax_ += this->pand_;
  this->panx_ = 1.0f;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::PanWest()
{
  this->panay_ += this->pand_;
  this->pany_ = 1.0f;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::PanSouth()
{
  this->panax_ -= this->pand_;
  this->panx_ = 1.0f;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::PanEast()
{
  this->panay_ -= this->pand_;
  this->pany_ = 1.0f;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::PanNorthWest() { this->PanNorth(); this->PanWest(); }

void
Canvas::PanNorthEast() { this->PanNorth(); this->PanEast(); }

void
Canvas::PanSouthWest() { this->PanSouth(); this->PanWest(); }

void
Canvas::PanSouthEast() { this->PanSouth(); this->PanEast(); }


void
Canvas::PushIn()
{
  this->dolly_.z += this->zoomd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::PullOut()
{
  this->dolly_.z -= this->zoomd_;
  glm::vec3 panvx = glm::vec3(1.0f);
  glm::vec3 panvy = glm::vec3(1.0f);
  if (this->panax_ != 0.0f) { panvx = glm::vec3(this->panx_, 0.0f, 0.0f); }
  if (this->panay_ != 0.0f) { panvy = glm::vec3(0.0f, this->pany_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panax_), panvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->panay_), panvy);
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}

