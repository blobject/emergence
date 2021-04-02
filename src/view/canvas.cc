#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Log& log, Control& ctrl,
               unsigned int width, unsigned int height, bool two)
  : ctrl_(ctrl), log_(log)
{
  ctrl.attach_to_state(*this);
  ctrl.attach_to_proc(*this);
  auto gui_state = GuiState(ctrl);
  this->gui_ = new Gui(log, gui_state, *this, width, height, two);
  if (nullptr == this->gui_->view_) {
    return; // TODO: handle error
  }
  this->gui_->set_pointer();
  glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit()) {
    log.add(Attn::E, "glewInit");
  }

  DOGL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  DOGL(glEnable(GL_DEPTH_TEST));
  DOGL(glEnable(GL_BLEND));
  DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  this->three_ = !two;
  this->levels_ = 50;
  this->level_ = 1;
  this->shift_counts_ = 5;
  this->shift_count_ = 0;
  this->ago_ = glfwGetTime();
  this->paused_ = false;

  this->width_ = 1000.0f;
  this->height_ = 1000.0f;
  this->zoomdef_ = -3.2f;
  this->neardef_ = -50.0f;
  this->dolly_ = glm::vec3(0.0f, 0.0f, this->zoomdef_);
  this->pivotax_ = 0.0f;
  this->pivotay_ = 0.0f;
  this->pivotx_ = 1.0f;
  this->pivoty_ = 1.0f;
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, this->dolly_);
  view = glm::rotate(view, glm::radians(this->pivotax_),
                     glm::vec3(this->pivotx_, this->pivoty_, 1.0f));
  this->view_ = view;
  this->orth_ = glm::ortho(0.0f, this->width_, 0.0f, this->height_,
                           this->neardef_, this->neardef_ + 100.0f);
  this->model_ = glm::mat4(1.0f);
  this->proj_ = glm::perspective(glm::radians(50.0f),
                                 this->width_ / this->height_, 0.1f, 100.0f);
  this->dollyd_ = 0.05f;
  this->pivotd_ = 1.0f;
  this->zoomd_ = 0.05f;

  this->shader_ = new Shader(this->log_);
  this->shader_->bind();
  this->camera_set();
  this->spawn();
}


Canvas::~Canvas()
{
  this->ctrl_.detach_from_state(*this);
  this->ctrl_.detach_from_proc(*this);
  delete this->vertex_buffer_xyz_;
  delete this->vertex_buffer_rgba_;
  delete this->vertex_buffer_quad_;
  delete this->vertex_array_;
  delete this->shader_;
}


void
Canvas::exec()
{
  Gui* gui = this->gui_;
  if (gui->closing()) {
    return;
  }


  /**
  // profiling
  GLuint64 start, stop;
  unsigned int queryID[2];
  glGenQueries(2, queryID);
  glQueryCounter(queryID[0], GL_TIMESTAMP);
  GLint stopAvailable = 0;
  bool profiled = false;
  //*/

  /**
  // timing
  double now;
  now = glfwGetTime();
  if (now - this->ago_ < 1.0f / 60.0f) {
    continue;
  }
  this->ago_ = now;
  //*/

  Control& ctrl = this->ctrl_;
  int num = ctrl.get_num();
  if (this->three_) {
    num *= this->level_;
  }

  this->clear();
  ctrl.coloring(this->ctrl_.get_coloring());
  this->draw(4, num, this->vertex_array_, this->shader_);
  // if paused, only particle processing (Proc) gets paused
  if (!this->paused_) {
    if (this->three_) {
      this->next3d();
    } else {
      this->next2d();
    }
  }
  gui->draw();
  gui->next();

  /**
  // profiling
  glQueryCounter(queryID[1], GL_TIMESTAMP);
  while (!stopAvailable) {
    glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE,
                       &stopAvailable);
  }
  if (!profiled) {
    profiled = true;
    glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
    glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
    std::cout << "time spent: " << (stop - start) / 1000000.0 << "\n";
  }
  //*/
}


void
Canvas::react(Issue issue)
{
  if (Issue::ProcNextDone == issue) {
    this->exec();
    return;
  } if (Issue::StateChanged == issue) {
    this->respawn();
    return;
  } if (Issue::ProcDone == issue) {
    this->paused_ = true;
  }
}


void
Canvas::spawn()
{
  State& state = this->ctrl_.get_state();
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<GLfloat>& xyz = this->xyz_;
  std::vector<GLfloat>& rgba = this->rgba_;
  GLfloat near = this->neardef_;
  for (int i = 0; i < state.num_; ++i) {
    /* x */ xyz.push_back(px[i]);
    /* y */ xyz.push_back(py[i]);
    /* z */ xyz.push_back(near);
    /* r */ rgba.push_back(xr[i]);
    /* g */ rgba.push_back(xg[i]);
    /* b */ rgba.push_back(xb[i]);
    /* a */ rgba.push_back(1.0f);
  }
  // TODO: dynamic particle radii
  GLfloat prad = state.prad_;
  GLfloat quad[] = {0.0f, prad,
                   -prad, 0.0f,
                    prad, 0.0f,
                    0.0f,-prad};

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  VertexBuffer* vb_xyz = new VertexBuffer(xyzarray, xyz.size()
                                          * sizeof(float));
  VertexBuffer* vb_rgba = new VertexBuffer(rgbaarray, rgba.size()
                                          * sizeof(float));
  VertexBuffer* vb_quad = new VertexBuffer(quad, sizeof(quad));
  VertexArray* va = new VertexArray();
  va->add_buffer(0, *vb_xyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
  va->add_buffer(1, *vb_rgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
  va->add_buffer(2, *vb_quad, VertexBufferAttribs::gen<GLfloat>(2, 2, 0));

  // instancing
  DOGL(glVertexAttribDivisor(0, 1));
  DOGL(glVertexAttribDivisor(1, 1));

  vb_xyz->unbind();
  vb_rgba->unbind();
  vb_quad->unbind();
  va->unbind();
  this->vertex_buffer_xyz_ = vb_xyz;
  this->vertex_buffer_rgba_ = vb_rgba;
  this->vertex_buffer_quad_ = vb_quad;
  this->vertex_array_ = va;
}


void
Canvas::respawn()
{
  delete this->vertex_buffer_xyz_;
  delete this->vertex_buffer_rgba_;
  delete this->vertex_buffer_quad_;
  delete this->vertex_array_;
  this->spawn();
  this->level_ = 1;
  this->shift_count_ = 0;
}


void
Canvas::draw(GLuint instances, GLuint instance_count, VertexArray* vertex_array,
             Shader* shader) const
{
  shader->bind();
  vertex_array->bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, instances,
                             instance_count));
}


void
Canvas::next2d()
{
  State& state = this->ctrl_.get_state();
  int num = state.num_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<GLfloat>& xyz = this->xyz_;
  std::vector<GLfloat>& rgba = this->rgba_;
  VertexBuffer* vb_xyz = this->vertex_buffer_xyz_;
  VertexBuffer* vb_rgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;

  // insert updated particle positions and colors into the buffer
  unsigned int xyzi = 0;
  unsigned int rgbai = 0;
  for (int p = 0; p < num; ++p) {
    xyz[xyzi++] = px[p];
    xyz[xyzi++] = py[p];
    xyz[xyzi++] = near;
    rgba[rgbai++] = xr[p];
    rgba[rgbai++] = xg[p];
    rgba[rgbai++] = xb[p];
    rgba[rgbai++] = 1.0f;
  }

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  vb_xyz->update(xyzarray, xyz.size() * sizeof(float));
  vb_rgba->update(rgbaarray, rgba.size() * sizeof(float));
  va->add_buffer(0, *vb_xyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
  va->add_buffer(1, *vb_rgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
}


void
Canvas::next3d()
{
  State &state = this->ctrl_.get_state();
  int num = state.num_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<GLfloat>& xyz = this->xyz_;
  std::vector<GLfloat>& rgba = this->rgba_;
  VertexBuffer* vb_xyz = this->vertex_buffer_xyz_;
  VertexBuffer* vb_rgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;
  bool shift = this->shift_count_ == this->shift_counts_;
  unsigned int xyzspan = 3 * num;
  unsigned int rgbaspan = 4 * num;
  unsigned int levels = this->levels_;
  unsigned int level = this->level_;

  if (shift) {
    xyz.resize(std::min(levels, level + 1) * xyzspan);
    rgba.resize(std::min(levels, level + 1) * rgbaspan);
  }

  // insert updated particle positions into the buffer
  unsigned int xyzi = 0;
  unsigned int rgbai = 0;
  for (int p = 0; p < num; ++p) {
    // shift particle levels (ie. represent passage of time via z & a)
    /* x */ this->shift(shift, level, px[p], 0.0f, xyz, xyzi, xyzspan);
    /* y */ this->shift(shift, level, py[p], 0.0f, xyz, xyzi, xyzspan);
    /* z */ this->shift(shift, level, near, 1.0f, xyz, xyzi, xyzspan);
    /* r */ this->shift(shift, level, xr[p], 0.0f, rgba, rgbai, rgbaspan);
    /* g */ this->shift(shift, level, xg[p], 0.0f, rgba, rgbai, rgbaspan);
    /* b */ this->shift(shift, level, xb[p], 0.0f, rgba, rgbai, rgbaspan);
    /* a */ this->shift(shift, level, 1.0f, -1.0f, rgba, rgbai, rgbaspan);
  }

  // restrict the number of levels
  if (shift && level < levels) {
    ++this->level_;
  }

  // control when the level shifting happens
  ++this->shift_count_;
  if (this->shift_count_ > this->shift_counts_) {
    this->shift_count_ = 0;
  }

  GLfloat* xyzarray = &xyz[0];
  GLfloat* rgbaarray = &rgba[0];
  vb_xyz->update(xyzarray, xyz.size() * sizeof(float));
  vb_rgba->update(rgbaarray, rgba.size() * sizeof(float));
  va->add_buffer(0, *vb_xyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
  va->add_buffer(1, *vb_rgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
}


void
Canvas::shift(bool shift, unsigned int level, GLfloat next, GLfloat d,
              std::vector<GLfloat>& v, unsigned int& i, unsigned int span)
{
  if (!shift) {
    return;
  }
  float shifted;
  // deeper levels
  for (unsigned int l = level; l >= 1; --l) {
    shifted = v[i + ((l - 1) * span)] + d;
    // encode alpha shift as a negative d
    if (d < 0) {
      shifted = 0.25;
    }
    v[i + (l * span)] = shifted;
  }
  // base level
  v[i++] = next;
}


void
Canvas::camera_default()
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
  this->camera_set();
}


void
Canvas::camera(GLfloat dx, GLfloat dy, GLfloat dz, GLfloat dax, GLfloat day)
{
  this->dolly_.x += dx;
  this->dolly_.y += dy;
  this->dolly_.z += dz;
  if (dax != 0.0f) {
    this->pivotax_ += dax; this->pivotx_ = 1.0f;
  }
  if (day != 0.0f) {
    this->pivotay_ += day; this->pivoty_ = 1.0f;
  }
  glm::vec3 pivotvx = glm::vec3(1.0f);
  glm::vec3 pivotvy = glm::vec3(1.0f);
  if (this->pivotax_ != 0.0f) {
    pivotvx = glm::vec3(this->pivotx_, 0.0f, 0.0f);
  }
  if (this->pivotay_ != 0.0f) {
    pivotvy = glm::vec3(0.0f, this->pivoty_, 0.0f);
  }
  this->view_ = glm::translate(glm::mat4(1.0f), this->dolly_);
  this->view_ = glm::rotate(this->view_, glm::radians(this->pivotax_),
                            pivotvx);
  this->view_ = glm::rotate(this->view_, glm::radians(this->pivotay_),
                            pivotvy);
  this->camera_set();
}


void
Canvas::camera_resize(GLfloat w, GLfloat h)
{
  DOGL(glViewport(0, 0, w, h));
  this->width_ = w;
  this->height_ = h;
  this->proj_ = glm::perspective(glm::radians(50.0f), w / h, 0.1f, 100.0f);
  this->camera_set();
}

