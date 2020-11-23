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
  this->levels_ = 10;
  this->level_ = 1;
  this->level_shift_counts_ = 5;
  this->level_shift_count_ = 0;
  this->ago_ = glfwGetTime();
  this->paused_ = false;
}


void
Canvas::Exec()
{
  DOGL(glEnable(GL_DEPTH_TEST));
  DOGL(glEnable(GL_BLEND));
  DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  Sys* sys = this->sys_;
  State &state = sys->state_;
  Gui* gui = this->gui_;

  unsigned int w = state.width_;
  unsigned int h = state.height_;

  GLfloat   zoomdef = -3.2f;
  GLfloat   neardef = -50.0f;
  glm::vec3 dolly = glm::vec3(0.0f, 0.0f, zoomdef);
  GLfloat   panax = 0.0f;
  GLfloat   panay = 0.0f;
  GLfloat   panx = 1.0f;
  GLfloat   pany = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, dolly);
  view = glm::rotate(view, glm::radians(panax), glm::vec3(panx, pany, 1.0f));
  glm::mat4 orth = glm::ortho(0.0f, (GLfloat)w, 0.0f, (GLfloat)h,
                              neardef, neardef + 100.0f);
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 proj = glm::perspective(glm::radians(50.0f),
                                    (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
  this->dolly_ = dolly;
  this->panax_ = panax;
  this->panay_ = panay;
  this->panx_ = panx;
  this->pany_ = pany;
  this->view_ = view;
  this->orth_ = orth;
  this->model_ = model;
  this->proj_ = proj;

  this->dollyd_ = 0.05f;
  this->pand_ = 1.0f;
  this->zoomd_ = 0.05f;
  this->zoomdef_ = zoomdef;
  this->neardef_ = neardef;

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
  this->shader_ = shader;
  shader->Bind();
  shader->SetUniform1f("prad", state.particles_.front().rad);
  this->CameraSet();

  // timing
  double now;

  // render loop
  while (! gui->Closing())
  {
    // timing
    /**
    now = glfwGetTime();
    if (now - this->ago_ < 1.0f / 60.0f)
    {
      continue;
    }
    this->ago_ = now;
    //*/

    if (!this->paused_)
    {
      this->Clear();
      this->Draw(4, this->level_ * state.num_, va, shader);
      sys->Next();
      this->Next();
    }

    gui->Draw();
    gui->Next();

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
  auto &particles = state.particles_;
  auto &ps = this->xyz_;
  auto &cs = this->rgba_;
  GLfloat near = this->neardef_;
  for (const Particle &particle : particles)
  {
    /* x */ ps.push_back(particle.x);
    /* y */ ps.push_back(particle.y);
    /* z */ ps.push_back(near);
    /* r */ cs.push_back(1.0f);
    /* g */ cs.push_back(1.0f);
    /* b */ cs.push_back(1.0f);
    /* a */ cs.push_back(1.0f);
  }
  GLfloat prad = particles.front().rad;
  GLfloat quad[] = { 0.0f, prad,
                    -prad, 0.0f,
                     prad, 0.0f,
                     0.0f,-prad };

  GLfloat* psarray = &ps[0];
  GLfloat* csarray = &cs[0];
  auto vbp = new VertexBuffer(psarray, ps.size() * sizeof(float));
  auto vbc = new VertexBuffer(csarray, cs.size() * sizeof(float));
  auto vbq = new VertexBuffer(quad, sizeof(quad));
  auto va = new VertexArray();
  va->AddBuffer(0, *vbp, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbc, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
  va->AddBuffer(2, *vbq, VertexBufferAttribs::Gen<GLfloat>(2, 2, 0));

  glVertexAttribDivisor(0, 1); // instancing

  vbp->Unbind();
  vbc->Unbind();
  vbq->Unbind();
  va->Unbind();
  this->vertex_buffer_xyz_ = vbp;
  this->vertex_buffer_rgba_ = vbc;
  this->vertex_buffer_quad_ = vbq;
  this->vertex_array_ = va;
}


// Respawn: Reinitialise OpenGL vertex constructs and redraw particles.

void
Canvas::Respawn()
{
  delete this->vertex_buffer_xyz_;
  delete this->vertex_buffer_rgba_;
  delete this->vertex_buffer_quad_;
  delete this->vertex_array_;
  this->Spawn();
}


void
Canvas::Draw(GLuint instances, GLuint instance_count, VertexArray* va,
             Shader* shader)
{
  shader->Bind();
  va->Bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, instances, instance_count));
}


void
Canvas::Clear()
{
  DOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}


void
Canvas::Next()
{
  auto &particles = this->sys_->state_.particles_;
  auto &ps = this->xyz_;
  auto &cs = this->rgba_;
  auto &vbp = this->vertex_buffer_xyz_;
  auto &vbc = this->vertex_buffer_rgba_;
  auto &va = this->vertex_array_;
  GLfloat near = this->neardef_;
  bool shift = this->level_shift_count_ == this->level_shift_counts_;
  int pspan = 3 * particles.size();
  int cspan = 4 * particles.size();
  int pindex = 0;
  int cindex = 0;
  unsigned int levels = this->levels_;
  unsigned int level = this->level_;

  if (shift)
  {
    ps.resize(std::min(levels, level + 1) * pspan);
    cs.resize(std::min(levels, level + 1) * cspan);
  }

  // insert updated particle positions into the buffer
  for (auto &particle : particles)
  {
    // shift particle levels (ie. represent passage of time via z & a)
    // x
    if (shift)
      for (int i = level; i >= 1; --i)
        ps[pindex + (i * pspan)] = ps[pindex + ((i - 1) * pspan)];
    ps[pindex++] = particle.x;

    // y
    if (shift)
      for (int i = level; i >= 1; --i)
        ps[pindex + (i * pspan)] = ps[pindex + ((i - 1) * pspan)];
    ps[pindex++] = particle.y;

    // z
    if (shift)
      for (int i = level; i >= 1; --i)
        ps[pindex + (i * pspan)] = ps[pindex + ((i - 1) * pspan)] + 1.0f;
    ps[pindex++] = near;

    // r
    if (shift)
      for (int i = level; i >= 1; --i)
        cs[cindex + (i * cspan)] = cs[cindex + ((i - 1) * cspan)];
    cs[cindex++] = 1.0f;

    // g
    if (shift)
      for (int i = level; i >= 1; --i)
        cs[cindex + (i * cspan)] = cs[cindex + ((i - 1) * cspan)];
    cs[cindex++] = 1.0f;

    // b
    if (shift)
      for (int i = level; i >= 1; --i)
        cs[cindex + (i * cspan)] = cs[cindex + ((i - 1) * cspan)];
    cs[cindex++] = 1.0f;

    // a
    if (shift)
      for (int i = level; i >= 1; --i)
        cs[cindex + (i * cspan)] =
          static_cast<float>(levels - level) / static_cast<float>(levels);
    cs[cindex++] = 1.0f;
  }

  /**
  for (int i = 0; i < ps.size(); ++i)
  {
    if (i % 7 == 0) std::cout << "\n" << i << ":";
    std::cout << ps[i] << ",";
  }
  std::cout << std::endl;
  //*/

  // restrict size of levels
  if (shift && level < levels)
  {
    ++this->level_;
  }

  // control when level shift happens
  ++this->level_shift_count_;
  if (this->level_shift_count_ > this->level_shift_counts_)
  {
    this->level_shift_count_ = 0;
  }

  GLfloat* psarray = &ps[0];
  GLfloat* csarray = &cs[0];
  vbp->Update(psarray, ps.size() * sizeof(float));
  vbc->Update(csarray, cs.size() * sizeof(float));
  va->AddBuffer(0, *vbp, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbc, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
}


void
Canvas::Pause()
{
  this->paused_ = !this->paused_;
}


void
Canvas::CameraSet()
{
  this->shader_->SetUniformMat4f(
    "mvp", this->proj_ * this->view_ * this->model_ * this->orth_);
}


void
Canvas::CameraDefault()
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
  this->orth_ = glm::ortho(0.0f, (GLfloat)w, 0.0f, (GLfloat)h,
                           this->neardef_, this->neardef_ + 100.0f);
  this->model_ = glm::mat4(1.0f);
  this->view_ = glm::rotate(view, glm::radians(this->panax_),
                            glm::vec3(this->panx_, this->pany_, 1.0f));
  this->proj_ = glm::perspective(glm::radians(50.0f),
                                 (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
  this->CameraSet();
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
  this->CameraSet();
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
  this->CameraSet();
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
  this->CameraSet();
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
  this->CameraSet();
}


void
Canvas::DollyNorthWest()
{
  this->DollyNorth();
  this->DollyWest();
  this->CameraSet();
}


void
Canvas::DollyNorthEast()
{
  this->DollyNorth();
  this->DollyEast();
  this->CameraSet();
}


void
Canvas::DollySouthWest()
{
  this->DollySouth();
  this->DollyWest();
  this->CameraSet();
}


void
Canvas::DollySouthEast()
{
  this->DollySouth();
  this->DollyEast();
  this->CameraSet();
}


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
  this->CameraSet();
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
  this->CameraSet();
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
  this->CameraSet();
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
  this->CameraSet();
}


void
Canvas::PanNorthWest()
{
  this->PanNorth();
  this->PanWest();
  this->CameraSet();
}


void
Canvas::PanNorthEast()
{
  this->PanNorth();
  this->PanEast();
  this->CameraSet();
}


void
Canvas::PanSouthWest()
{
  this->PanSouth();
  this->PanWest();
  this->CameraSet();
}


void
Canvas::PanSouthEast()
{
  this->PanSouth();
  this->PanEast();
  this->CameraSet();
}


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
  this->CameraSet();
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
  this->CameraSet();
}

