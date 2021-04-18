#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Log& log, Control& ctrl, UiState& uistate,
               bool gui_on, bool three)
  : ctrl_(ctrl), log_(log), gui_on_(gui_on), three_(three)
{
  ctrl.attach_to_state(*this);
  ctrl.attach_to_proc(*this);

  this->width_ = static_cast<GLfloat>(1000.0f);
  this->height_ = static_cast<GLfloat>(1000.0f);
  float window_scale = DPI / 100.0f;

  this->preamble(this->width_ / window_scale, this->height_ / window_scale);

  if (gui_on) {
    this->gui_ = new Gui(log, uistate, *this, this->window_, window_scale,
                         three);
    this->ago_ = glfwGetTime();
  }

  State& state = ctrl.get_state();

  this->levels_ = 50;
  this->level_ = 1;
  this->trail_ = false;
  this->trail_count_ = 0;
  this->trail_end_ = false;
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
  this->orth_ = glm::ortho(0.0f, static_cast<GLfloat>(state.width_),
                           0.0f, static_cast<GLfloat>(state.height_),
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

  log.add(Attn::O, "Started canvas module.", true);
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
  DOGL(glEnable(GL_BLEND));
  DOGL(glBlendEquation(GL_FUNC_ADD));
  DOGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  // leave off depth test for proper alpha overlapping, and take care of buffer
  // data ordering ourselves (newest/foremost should be drawn last)
  //DOGL(glEnable(GL_DEPTH_TEST));
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
  } else if (this->trail_) {
    num *= this->trail_count_ + 1;
  }

  this->clear();
  ctrl.color(ctrl.get_coloring());
  this->draw(4, num, this->vertex_array_, this->shader_);
  if (this->gui_on_) {
    this->gui_->draw();
  }
  // if paused, only particle processing (Proc) gets paused
  if (!ctrl.paused_ || ctrl.step_) {
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
    // empty
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
  std::vector<float>& xa = state.xa_;
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
    rgba.push_back(xa[i]);
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
  this->trail_count_ = 0;
  this->trail_end_ = false;
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
  unsigned int num = state.num_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<float>& xa = state.xa_;
  std::vector<GLfloat>& xyz = this->xyz_;
  std::vector<GLfloat>& rgba = this->rgba_;
  VertexBuffer* vb_xyz = this->vertex_buffer_xyz_;
  VertexBuffer* vb_rgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;
  unsigned int xyzspan = 3 * num;
  unsigned int rgbaspan = 4 * num;
  bool end = this->trail_end_;
  unsigned int trail;
  unsigned int xyznextstride;
  unsigned int xyzstride;
  unsigned int rgbastride;
  unsigned int xyzi;
  unsigned int rgbai;

  // leave a trail
  // Important: "newer" data should be situated towards the back of the list,
  //            so that OpenGL will draw them later and thereby represent depth
  //            and alpha blend correctly.
  if (this->trail_) {
    if (!end) {
      ++this->trail_count_;
    }
    trail = this->trail_count_;
    if (end) {
      for (unsigned int t = 0; t < trail; ++t) {
        xyzi = 0;
        for (int i = 0; i < num; ++i) {
          xyzstride = t * xyzspan;
          xyznextstride = (t + 1) * xyzspan;
          xyz[xyzstride + xyzi] = xyz[xyznextstride + xyzi]; ++xyzi;
          xyz[xyzstride + xyzi] = xyz[xyznextstride + xyzi]; ++xyzi;
          ++xyzi; // z does not change
          // rgba do not change
        }
      }
    } else {
      xyz.resize((trail + 1) * xyzspan);
      rgba.resize((trail + 1) * rgbaspan);
      rgbastride = (trail - 1) * rgbaspan;
      rgbai = 0;
      for (unsigned int i = 0; i < num; ++i) {
        // xyz do not change
        rgba[rgbastride + rgbai++] = 0.3f;
        rgba[rgbastride + rgbai++] = 0.3f;
        rgba[rgbastride + rgbai++] = 0.3f;
        rgba[rgbastride + rgbai++] = 0.05f;
      }
    }
    if (150 <= trail) {
      this->trail_end_ = true;
    }
  }
  trail = this->trail_count_;

  // insert updated particle positions and colors (at trail tail)
  xyzstride = trail * xyzspan;
  rgbastride = trail * rgbaspan;
  xyzi = 0;
  rgbai = 0;
  for (int i = 0; i < num; ++i) {
    xyz[xyzstride + xyzi++] = px[i];
    xyz[xyzstride + xyzi++] = py[i];
    xyz[xyzstride + xyzi++] = near;
    rgba[rgbastride + rgbai++] = xr[i];
    rgba[rgbastride + rgbai++] = xg[i];
    rgba[rgbastride + rgbai++] = xb[i];
    rgba[rgbastride + rgbai++] = xa[i];
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
  unsigned int num = state.num_;
  std::vector<float>& px = state.px_;
  std::vector<float>& py = state.py_;
  std::vector<float>& xr = state.xr_;
  std::vector<float>& xg = state.xg_;
  std::vector<float>& xb = state.xb_;
  std::vector<float>& xa = state.xa_;
  std::vector<GLfloat>& xyz = this->xyz_;
  std::vector<GLfloat>& rgba = this->rgba_;
  VertexBuffer* vb_xyz = this->vertex_buffer_xyz_;
  VertexBuffer* vb_rgba = this->vertex_buffer_rgba_;
  VertexArray* va = this->vertex_array_;
  GLfloat near = this->neardef_;
  bool end = (this->levels_ <= this->level_);
  bool shift = this->shift_counts_ <= this->shift_count_;
  unsigned int xyzspan = 3 * num;
  unsigned int rgbaspan = 4 * num;
  unsigned int xyzstride;
  unsigned int rgbastride;
  unsigned int xyznextstride;
  unsigned int rgbanextstride;
  unsigned int xyzi;
  unsigned int rgbai;
  unsigned int level;

  // shift particle levels
  if (shift) {
    if (!end) {
      ++this->level_;
    }
    level = this->level_;
    if (end) {
      for (unsigned int l = 0; l < level; ++l) {
        xyzstride = l * xyzspan;
        rgbastride = l * rgbaspan;
        xyznextstride = (l + 1) * xyzspan;
        rgbanextstride = (l + 1) * rgbaspan;
        xyzi = 0;
        rgbai = 0;
        for (unsigned int i = 0; i < num; ++i) {
          xyz[xyzstride + xyzi] = xyz[xyznextstride + xyzi]; ++xyzi;
          xyz[xyzstride + xyzi] = xyz[xyznextstride + xyzi]; ++xyzi;
          xyz[xyzstride + xyzi] = xyz[xyznextstride + xyzi] + 1.0f; ++xyzi;
          rgba[rgbastride + rgbai] = rgba[rgbanextstride + rgbai]; ++rgbai;
          rgba[rgbastride + rgbai] = rgba[rgbanextstride + rgbai]; ++rgbai;
          rgba[rgbastride + rgbai] = rgba[rgbanextstride + rgbai]; ++rgbai;
          rgba[rgbastride + rgbai++] = 0.1f;
        }
      }
    } else {
      xyz.resize(level * xyzspan);
      rgba.resize(level * rgbaspan);
      for (unsigned int l = 0; l < level - 1; ++l) {
        xyzstride = l * xyzspan;
        rgbastride = l * rgbaspan;
        xyzi = 0;
        rgbai = 0;
        for (unsigned int i = 0; i < num; ++i) {
          xyzi += 2;
          xyz[xyzstride + xyzi++] += 1.0f;
          rgbai += 3;
          rgba[rgbastride + rgbai++] = 0.1f;
        }
      }
    }
  }
  ++this->shift_count_;
  if (this->shift_counts_ < this->shift_count_) {
    this->shift_count_ = 0;
  }

  // insert updated particle positions and colors into the buffer
  level = this->level_;
  xyzstride = (level - 1) * xyzspan;
  rgbastride = (level - 1) * rgbaspan;
  xyzi = 0;
  rgbai = 0;
  for (unsigned int p = 0; p < num; ++p) {
    xyz[xyzstride + xyzi++] = px[p];
    xyz[xyzstride + xyzi++] = py[p];
    xyz[xyzstride + xyzi++] = near;
    rgba[rgbastride + rgbai++] = xr[p];
    rgba[rgbastride + rgbai++] = xg[p];
    rgba[rgbastride + rgbai++] = xb[p];
    rgba[rgbastride + rgbai++] = xa[p];
  }

  GLfloat* p_xyz = &xyz[0];
  GLfloat* p_rgba = &rgba[0];
  vb_xyz->update(p_xyz, xyz.size() * sizeof(float));
  vb_rgba->update(p_rgba, rgba.size() * sizeof(float));
  va->add_buffer(0, *vb_xyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
  va->add_buffer(1, *vb_rgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
}


void
Canvas::camera_default()
{
  State& state = this->ctrl_.get_state();
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
  this->orth_ = glm::ortho(0.0f, static_cast<GLfloat>(state.width_),
                           0.0f, static_cast<GLfloat>(state.height_),
                           this->neardef_, this->neardef_ + 100.0f);
  this->proj_ = glm::perspective(glm::radians(50.0f),
                                 this->width_ / this->height_, 0.1f, 100.0f);
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
  State& state = this->ctrl_.get_state();
  this->width_ = w;
  this->height_ = h;
  this->orth_ = glm::ortho(0.0f, static_cast<GLfloat>(state.width_),
                           0.0f, static_cast<GLfloat>(state.height_),
                           this->neardef_, this->neardef_ + 100.0f);
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
  this->ctrl_.quit();
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
  Control& ctrl = canvas->ctrl_;

  if (GLFW_RELEASE == action) {
    return;
  }
  if (GLFW_PRESS == action) {
    if (mods & GLFW_MOD_CONTROL) {
      if (GLFW_KEY_C == key || GLFW_KEY_Q == key ) {
        canvas->close();
        return;
      }
    }
    if (GLFW_KEY_SPACE == key) {
      ctrl.pause(!ctrl.paused_);
      return;
    }
  }
  if (GLFW_KEY_S == key) {
    ctrl.paused_ = true;
    ctrl.step_ = true;
    return;
  }
}


void
Canvas::resize_callback(GLFWwindow* window, int w, int h)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  canvas->camera_resize(w, h);
}

