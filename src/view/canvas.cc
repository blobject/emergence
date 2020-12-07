#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Log &log, State &state, Proc &proc, bool hide_ctrl)
  : log_(log), state_(state), proc_(proc), shader_(NULL)
{
  state.Attach(*this);
  auto gui_state = GuiState(state);
  this->gui_ = new Gui(log, gui_state, *this, "#version 330 core",
                       state.width_, state.height_, hide_ctrl);
  if (nullptr == this->gui_->view_)
  {
    return; // TODO: handle error
  }
  this->gui_->SetPointer();
	glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit())
  {
    log.Add(Attn::E, "glewInit");
  }
  this->width_ = 1000.0f;
  this->height_ = 1000.0f;
  this->three_ = true;
  this->levels_ = 50;
  this->level_ = 1;
  this->shift_counts_ = 5;
  this->shift_count_ = 0;
  this->ago_ = glfwGetTime();
  this->hard_paused_ = false;
  this->paused_ = false;
}


Canvas::~Canvas()
{
  this->state_.Detach(*this);
}


void
Canvas::React(Subject &next_state)
{
  this->Respawn();
}


void
Canvas::Exec()
{
  DOGL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  DOGL(glEnable(GL_DEPTH_TEST));
  DOGL(glEnable(GL_BLEND));
  DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  Proc &proc = this->proc_;
  State &state = this->state_;
  Gui* gui = this->gui_;
  GLfloat   width = this->width_;
  GLfloat   height = this->height_;
  GLfloat   zoomdef = -3.2f;
  GLfloat   neardef = -50.0f;
  glm::vec3 dolly = glm::vec3(0.0f, 0.0f, zoomdef);
  GLfloat   pivotax = 0.0f;
  GLfloat   pivotay = 0.0f;
  GLfloat   pivotx = 1.0f;
  GLfloat   pivoty = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, dolly);
  view = glm::rotate(view, glm::radians(pivotax),
                     glm::vec3(pivotx, pivoty, 1.0f));
  glm::mat4 orth = glm::ortho(0.0f, width, 0.0f, height,
                              neardef, neardef + 100.0f);
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 proj = glm::perspective(glm::radians(50.0f),
                                    width / height, 0.1f, 100.0f);
  this->dolly_ = dolly;
  this->pivotax_ = pivotax;
  this->pivotay_ = pivotay;
  this->pivotx_ = pivotx;
  this->pivoty_ = pivoty;
  this->view_ = view;
  this->orth_ = orth;
  this->model_ = model;
  this->proj_ = proj;

  this->dollyd_ = 0.05f;
  this->pivotd_ = 1.0f;
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

  unsigned int num;
  this->shader_ = new Shader(this->log_);
  this->shader_->Bind();
  //shader->SetUniform1f("prad", state.prad_.front());
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

    num = state.num_;
    if (this->three_)
    {
      num = this->level_ * state.num_;
    }

    if (! this->hard_paused_)
    {
      this->Clear();
      this->Draw(4, num, this->vertex_array_, this->shader_);
      if (! this->paused_)
      {
        proc.Next(); // changes particles' X, Y, F, N, etc.
        if (this->three_) { this->Next3d(); }
        else              { this->Next2d(); }
      }
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
  State &state = this->state_;
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
  GLfloat quad[] = {0.0f, prad,
                   -prad, 0.0f,
                    prad, 0.0f,
                    0.0f,-prad};

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  VertexBuffer* vbxyz = new VertexBuffer(xyzarray, xyz.size() * sizeof(float));
  VertexBuffer* vbrgba = new VertexBuffer(rgbaarray, rgba.size() * sizeof(float));
  VertexBuffer* vbquad = new VertexBuffer(quad, sizeof(quad));
  VertexArray* va = new VertexArray();
  va->AddBuffer(0, *vbxyz, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbrgba, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
  va->AddBuffer(2, *vbquad, VertexBufferAttribs::Gen<GLfloat>(2, 2, 0));

  // instancing
  glVertexAttribDivisor(0, 1);
  glVertexAttribDivisor(1, 1);

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
  this->level_ = 1;
  this->shift_count_ = 0;
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
Canvas::Next2d()
{
  State &state = this->state_;
  unsigned int num = state.num_;
  GLfloat scope = state.scope_;
  std::vector<float> &px = state.px_;
  std::vector<float> &py = state.py_;
  std::vector<unsigned int> &pn = state.pn_;
  std::vector<GLfloat> &xyz = this->xyz_;
  std::vector<GLfloat> &rgba = this->rgba_;
  VertexBuffer* vbxyz = this->vertex_buffer_xyz_;
  VertexBuffer* vbrgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;

  // insert updated particle positions into the buffer
  unsigned int xyzi = 0;
  unsigned int rgbai = 0;
  for (unsigned int p = 0; p < num; ++p)
  {
    xyz[xyzi++] = px[p];
    xyz[xyzi++] = py[p];
    xyz[xyzi++] = near;
    rgba[rgbai++] = static_cast<GLfloat>(pn[p]) / (scope / 1.5f);
    rgba[rgbai++] = static_cast<GLfloat>(pn[p]) / scope;
    rgba[rgbai++] = 0.7f;
    rgba[rgbai++] = 1.0f;
  }

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  vbxyz->Update(xyzarray, xyz.size() * sizeof(float));
  vbrgba->Update(rgbaarray, rgba.size() * sizeof(float));
  va->AddBuffer(0, *vbxyz, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbrgba, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
}


void
Canvas::Next3d()
{
  State &state = this->state_;
  unsigned int num = state.num_;
  GLfloat scope = state.scope_;
  std::vector<float> &px = state.px_;
  std::vector<float> &py = state.py_;
  std::vector<unsigned int> &pn = state.pn_;
  std::vector<GLfloat> &xyz = this->xyz_;
  std::vector<GLfloat> &rgba = this->rgba_;
  VertexBuffer* vbxyz = this->vertex_buffer_xyz_;
  VertexBuffer* vbrgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;
  bool shift = this->shift_count_ == this->shift_counts_;
  unsigned int xyzspan = 3 * num;
  unsigned int rgbaspan = 4 * num;
  unsigned int levels = this->levels_;
  unsigned int level = this->level_;

  if (shift)
  {
    xyz.resize(std::min(levels, level + 1) * xyzspan);
    rgba.resize(std::min(levels, level + 1) * rgbaspan);
  }

  // insert updated particle positions into the buffer
  unsigned int xyzi = 0;
  unsigned int rgbai = 0;
  for (unsigned int p = 0; p < num; ++p)
  {
    // shift particle levels (ie. represent passage of time via z & a)
    /* x */ this->Shift(shift, level, px[p], 0.0f, xyz, xyzi, xyzspan);
    /* y */ this->Shift(shift, level, py[p], 0.0f, xyz, xyzi, xyzspan);
    /* z */ this->Shift(shift, level, near, 1.0f, xyz, xyzi, xyzspan);
    /* r */ this->Shift(shift, level,
                        static_cast<GLfloat>(pn[p]) / (scope / 1.5f),
                        0.0f, rgba, rgbai, rgbaspan);
    /* g */ this->Shift(shift, level,
                        static_cast<GLfloat>(pn[p]) / scope,
                        0.0f, rgba, rgbai, rgbaspan);
    /* b */ this->Shift(shift, level, 0.7f, 0.0f, rgba, rgbai, rgbaspan);
    /* a */ this->Shift(shift, level, 1.0f, -1.0f, rgba, rgbai, rgbaspan);
  }

  // restrict size of levels
  if (shift && level < levels)
  {
    ++this->level_;
  }

  // control when level shift happens
  ++this->shift_count_;
  if (this->shift_count_ > this->shift_counts_)
  {
    this->shift_count_ = 0;
  }

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  vbxyz->Update(xyzarray, xyz.size() * sizeof(float));
  vbrgba->Update(rgbaarray, rgba.size() * sizeof(float));
  va->AddBuffer(0, *vbxyz, VertexBufferAttribs::Gen<GLfloat>(3, 3, 0));
  va->AddBuffer(1, *vbrgba, VertexBufferAttribs::Gen<GLfloat>(4, 4, 0));
}


void
Canvas::Shift(bool shift, unsigned int level, GLfloat next, GLfloat inc,
              std::vector<GLfloat> &v, unsigned int &i, unsigned int span)
{
  if (! shift)
  {
    return;
  }
  float shifted;
  // deeper levels
  for (unsigned int l = level; l >= 1; --l)
  {
    shifted = v[i + ((l - 1) * span)] + inc;
    // encode alpha shift as a negative d
    if (inc < 0)
    {
      shifted = 0.25;
    }
    v[i + (l * span)] = shifted;
  }
  // base level
  v[i++] = next;
}


void
Canvas::Three(bool yes)
{
  this->three_ = yes;
}


void
Canvas::HardPause(bool yes)
{
  this->hard_paused_ = yes;
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
  GLfloat w = this->width_;
  GLfloat h = this->height_;
  this->dolly_ = glm::vec3(0.0f, 0.0f, this->zoomdef_);
  this->pivotax_ = 0.0f;
  this->pivotay_ = 0.0f;
  this->pivotx_ = 1.0f;
  this->pivoty_ = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, this->dolly_);
  this->model_ = glm::mat4(1.0f);
  this->view_ = glm::rotate(view, glm::radians(this->pivotax_),
                            glm::vec3(this->pivotx_, this->pivoty_, 1.0f));
  this->proj_ = glm::perspective(glm::radians(50.0f), w / h, 0.1f, 100.0f);
  this->CameraSet();
}


void
Canvas::Camera(GLfloat dx, GLfloat dy, GLfloat dz, GLfloat dax, GLfloat day)
{
  this->dolly_.x += dx;
  this->dolly_.y += dy;
  this->dolly_.z += dz;
  if (dax != 0.0f) { this->pivotax_ += dax; this->pivotx_ = 1.0f; }
  if (day != 0.0f) { this->pivotay_ += day; this->pivoty_ = 1.0f; }
  glm::vec3 pivotvx = glm::vec3(1.0f);
  glm::vec3 pivotvy = glm::vec3(1.0f);
  if (this->pivotax_ != 0.0f) { pivotvx = glm::vec3(this->pivotx_, 0.0f, 0.0f); }
  if (this->pivotay_ != 0.0f) { pivotvy = glm::vec3(0.0f, this->pivoty_, 0.0f); }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->pivotax_), pivotvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->pivotay_), pivotvy);
  this->CameraSet();
}


void
Canvas::CameraResize(GLfloat w, GLfloat h)
{
  DOGL(glViewport(0, 0, w, h));
  this->width_ = w;
  this->height_ = h;
  this->proj_ = glm::perspective(glm::radians(50.0f), w / h, 0.1f, 100.0f);
  this->CameraSet();
}

