#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Proc* proc, bool hide_ctrl)
  : proc_(proc), shader_(NULL)
{
  State &state = proc->state_;
  auto gui_state = GuiState(state);
  this->gui_ = new Gui(gui_state, proc, "#version 330 core", state.width_,
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
  this->levels_ = 50;
  this->level_ = 1;
  this->level_shift_counts_ = 5;
  this->level_shift_count_ = 0;
  this->ago_ = glfwGetTime();
  this->paused_ = false;
}


void
Canvas::Exec()
{
  DOGL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
  DOGL(glEnable(GL_DEPTH_TEST));
  DOGL(glEnable(GL_BLEND));
  //DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  DOGL(glBlendFunc(GL_ONE, GL_ONE));
  DOGL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));

  Proc* proc = this->proc_;
  State &state = proc->state_;
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
  shader->SetUniform1f("prad", state.prad_.front());
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

    if (! this->paused_)
    {
      this->Clear();
      this->Draw(4, this->level_ * state.num_, va, shader);
      //this->Draw(4, state.num_, va, shader); // DEBUG: draw 2d
      proc->Next();
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
  State &state = this->proc_->state_;
  auto &xyz = this->xyz_;
  auto &rgba = this->rgba_;
  GLfloat near = this->neardef_;
  for (int i = 0; i < state.num_; ++i)
  {
    /* x */ xyz.push_back(state.px_[i]);
    /* y */ xyz.push_back(state.py_[i]);
    /* z */ xyz.push_back(near);
    /* r */ rgba.push_back(1.0f);
    /* g */ rgba.push_back(1.0f);
    /* b */ rgba.push_back(1.0f);
    /* a */ rgba.push_back(1.0f);
  }
  // TODO: dynamic particle radii
  GLfloat prad = state.prad_.front();
  GLfloat quad[] = { 0.0f, prad,
                    -prad, 0.0f,
                     prad, 0.0f,
                     0.0f,-prad };

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  VertexBuffer* vbxyz = new VertexBuffer(xyzarray, xyz.size() * sizeof(float));
  VertexBuffer* vbrgba = new VertexBuffer(rgbaarray, rgba.size() * sizeof(float));
  VertexBuffer* vbquad = new VertexBuffer(quad, sizeof(quad));
  VertexArray* va = new VertexArray();
  va->AddBuffer(0, *vbxyz, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbrgba, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
  va->AddBuffer(2, *vbquad, VertexBufferAttribs::Gen<GLfloat>(2, 2, 0));

  glVertexAttribDivisor(0, 1); // instancing

  vbxyz->Unbind();
  vbrgba->Unbind();
  vbquad->Unbind();
  va->Unbind();
  this->vertex_buffer_xyz_ = vbxyz;
  this->vertex_buffer_rgba_ = vbrgba;
  this->vertex_buffer_quad_ = vbquad;
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
  State &state = this->proc_->state_;
  unsigned int num = state.num_;
  std::vector<float> &px = state.px_;
  std::vector<float> &py = state.py_;
  std::vector<GLfloat> &xyz = this->xyz_;
  std::vector<GLfloat> &rgba = this->rgba_;
  VertexBuffer* vbxyz = this->vertex_buffer_xyz_;
  VertexBuffer* vbrgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;
  bool shift = this->level_shift_count_ == this->level_shift_counts_;
  int xyzspan = 3 * num;
  int rgbaspan = 4 * num;
  int xyzi = 0;
  int rgbai = 0;
  unsigned int levels = this->levels_;
  unsigned int level = this->level_;

  if (shift)
  {
    xyz.resize(std::min(levels, level + 1) * xyzspan);
    rgba.resize(std::min(levels, level + 1) * rgbaspan);
  }

  // insert updated particle positions into the buffer
  for (int p = 0; p < num; ++p)
  {
    // shift particle levels (ie. represent passage of time via z & a)
    // x
    if (shift)
      for (int l = level; l >= 1; --l)
        xyz[xyzi + (l * xyzspan)] = xyz[xyzi + ((l - 1) * xyzspan)];
    xyz[xyzi++] = px[p];

    // y
    if (shift)
      for (int l = level; l >= 1; --l)
        xyz[xyzi + (l * xyzspan)] = xyz[xyzi + ((l - 1) * xyzspan)];
    xyz[xyzi++] = py[p];

    // z
    if (shift)
      for (int l = level; l >= 1; --l)
        xyz[xyzi + (l * xyzspan)] = xyz[xyzi + ((l - 1) * xyzspan)] + 1.0f;
    xyz[xyzi++] = near;

    // r
    if (shift)
      for (int l = level; l >= 1; --l)
        rgba[rgbai + (l * rgbaspan)] = rgba[rgbai + ((l - 1) * rgbaspan)];
    rgba[rgbai++] = 1.0f;

    // g
    if (shift)
      for (int l = level; l >= 1; --l)
        rgba[rgbai + (l * rgbaspan)] = rgba[rgbai + ((l - 1) * rgbaspan)];
    rgba[rgbai++] = 1.0f;

    // b
    if (shift)
      for (int l = level; l >= 1; --l)
        rgba[rgbai + (l * rgbaspan)] = rgba[rgbai + ((l - 1) * rgbaspan)];
    rgba[rgbai++] = 1.0f;

    // a
    if (shift)
      for (int l = level; l >= 1; --l)
        rgba[rgbai + (l * rgbaspan)] =
          0.5f - static_cast<float>(level) / levels / 2;
    rgba[rgbai++] = 1.0f;
  }

  /**
  for (int i = 0; i < rgba.size(); ++i)
  {
    if (i % 4 == 0) std::cout << "\n" << i << ":";
    std::cout << rgba[i] << ",";
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

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  vbxyz->Update(xyzarray, xyz.size() * sizeof(float));
  vbrgba->Update(rgbaarray, rgba.size() * sizeof(float));
  va->AddBuffer(0, *vbxyz, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbrgba, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
}


void
Canvas::Pause()
{
  this->paused_ = ! this->paused_;
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
  unsigned int w = this->proc_->state_.width_;
  unsigned int h = this->proc_->state_.height_;
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

