#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Log& log, Control& ctrl,
               unsigned int width, unsigned int height,
               bool gui_on, bool three)
  : ctrl_(ctrl), log_(log), gui_on_(gui_on), three_(three)
{
  ctrl.attach_to_state(*this);
  ctrl.attach_to_proc(*this);

  this->width_ = static_cast<GLfloat>(width);
  this->height_ = static_cast<GLfloat>(height);
  float window_scale = DPI / 100.0f;

  this->preamble(this->width_ / window_scale, this->height_ / window_scale);

  if (gui_on) {
    auto gui_state = GuiState(ctrl);
    this->gui_ = new Gui(log, gui_state, *this, this->window_, window_scale,
                         three);
    this->ago_ = glfwGetTime();
  }

  this->paused_ = false;
  this->levels_ = 50;
  this->level_ = 1;
  this->shift_counts_ = 5;
  this->shift_count_ = 0;

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


void
Canvas::preamble(unsigned int window_width, unsigned int window_height)
{
  // glfw
  GLFWwindow* window;
  if (!glfwInit()) {
    this->log_.add(Attn::Egl, "glfwInit");
    return;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(window_width, window_height, ME, NULL, NULL);
  if (!window) {
    this->log_.add(Attn::Egl, "glfwCreateWindow");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetWindowSizeCallback(window, resize_callback);
  if (!this->gui_on_) {
    glfwSetKeyCallback(window, key_callback_no_gui);
  }
  this->window_ = window;
  this->set_pointer();

  // glew
  glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit()) {
    this->log_.add(Attn::E, "glewInit");
  }

  // gl
  DOGL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  DOGL(glEnable(GL_DEPTH_TEST));
  DOGL(glEnable(GL_BLEND));
  DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
  if (this->closing()) {
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
  ctrl.coloring(ctrl.get_coloring());
  this->draw(4, num, this->vertex_array_, this->shader_);
  if (this->gui_on_) {
    this->gui_->draw();
  }
  // if paused, only particle processing (Proc) gets paused
  if (!this->paused_) {
    if (this->three_) {
      this->next3d();
    } else {
      this->next2d();
    }
  }
  this->next();

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
    xyz.push_back(px[i]);
    xyz.push_back(py[i]);
    xyz.push_back(near);
    rgba.push_back(xr[i]);
    rgba.push_back(xg[i]);
    rgba.push_back(xb[i]);
    rgba.push_back(1.0f);
  }
  GLfloat prad = state.prad_;
  GLfloat quad[] = {0.0f, prad,
                   -prad, 0.0f,
                    prad, 0.0f,
                    0.0f,-prad};

  GLfloat* p_xyz = &xyz[0];
  GLfloat* p_rgba = &rgba[0];
  VertexBuffer* vb_xyz = new VertexBuffer(p_xyz, xyz.size() * sizeof(float));
  VertexBuffer* vb_rgba = new VertexBuffer(p_rgba, rgba.size()
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

  // in case state changed but canvas is paused
  if (this->three_) {
    this->next3d();
  } else {
    this->next2d();
  }
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
Canvas::draw(GLuint instances, GLuint instance_count,
             VertexArray* vertex_array, Shader* shader) const
{
  shader->bind();
  vertex_array->bind();
  DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, instances, instance_count));
}


void
Canvas::next2d()
{
  State& state = this->ctrl_.get_state();
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
  for (int i = 0; i < state.num_; ++i) {
    xyz[xyzi++] = px[i];
    xyz[xyzi++] = py[i];
    xyz[xyzi++] = near;
    rgba[rgbai++] = xr[i];
    rgba[rgbai++] = xg[i];
    rgba[rgbai++] = xb[i];
    rgba[rgbai++] = 1.0f;
  }

  GLfloat* p_xyz = &xyz[0];
  GLfloat* p_rgba = &rgba[0];
  vb_xyz->update(p_xyz, xyz.size() * sizeof(float));
  vb_rgba->update(p_rgba, rgba.size() * sizeof(float));
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
  for (int i = 0; i < num; ++i) {
    // shift particle levels (ie. represent passage of time via z & a)
    this->shift(shift, level, px[i], 0.0f, xyz, xyzi, xyzspan);
    this->shift(shift, level, py[i], 0.0f, xyz, xyzi, xyzspan);
    this->shift(shift, level, near,  1.0f, xyz, xyzi, xyzspan);
    this->shift(shift, level, xr[i], 0.0f, rgba, rgbai, rgbaspan);
    this->shift(shift, level, xg[i], 0.0f, rgba, rgbai, rgbaspan);
    this->shift(shift, level, xb[i], 0.0f, rgba, rgbai, rgbaspan);
    this->shift(shift, level, 1.0f, -1.0f, rgba, rgbai, rgbaspan);
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

  GLfloat* p_xyz = &xyz[0];
  GLfloat* p_rgba = &rgba[0];
  vb_xyz->update(p_xyz, xyz.size() * sizeof(float));
  vb_rgba->update(p_rgba, rgba.size() * sizeof(float));
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


void
Canvas::next() const
{
  glfwSwapBuffers(this->window_);
  glfwPollEvents();
}


void
Canvas::set_pointer()
{
  // for KeyCallback to access gui
  glfwSetWindowUserPointer(this->window_, this);
}


void
Canvas::close()
{
  glfwSetWindowShouldClose(this->window_, true);
  this->quit();
}


bool
Canvas::closing() const
{
  return glfwWindowShouldClose(this->window_);
}


void
Canvas::key_callback_no_gui(
  GLFWwindow* window, int key, int /* scancode */, int action, int mods
) {
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));

  if (action == GLFW_PRESS) {
    if (mods & GLFW_MOD_CONTROL) {
      if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
        canvas->close();
        return;
      }
    }
    if (key == GLFW_KEY_SPACE) {
      canvas->pause();
      return;
    }
  }
}


void
Canvas::resize_callback(GLFWwindow* window, int w, int h)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  canvas->camera_resize(w, h);
}

