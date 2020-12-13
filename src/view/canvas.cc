#include "canvas.hh"
#include "../util/util.hh"


Canvas::Canvas(Log& log, Control& ctrl,
               unsigned int width, unsigned int height, bool hide_ctrl)
    : log_(log), ctrl_(ctrl), shader_(NULL)
{
    ctrl.attach_to_state(*this);
    ctrl.attach_to_proc(*this);
    auto gui_state = GuiState(ctrl);
    this->gui_ = new Gui(log, gui_state, *this, width, height, hide_ctrl);
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

    State& state = this->ctrl_.get_state();
    this->three_ = true;
    this->levels_ = 50;
    this->level_ = 1;
    this->shift_counts_ = 5;
    this->shift_count_ = 0;
    this->ago_ = glfwGetTime();
    this->hard_paused_ = false;
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
    //shader->SetUniform1f("prad", state.prad_.front());
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


// exec: Render one iteration of all the graphics.

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

    int num = this->ctrl_.get_num();
    if (this->three_) {
        num *= this->level_;
    }

    // both hard and soft paused states pause Proc
    // if hard paused, only the GUI is updated
    if (!this->hard_paused_) {
        this->clear();
        this->draw(4, num, this->vertex_array_, this->shader_);
        // if (soft) paused, everything updates except the particles
        if (!this->paused_) {
            if (this->three_) {
                this->next3d();
            } else {
                this->next2d();
            }
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


// react: Canvas observes State::change(), Proc::next(), Proc::done().

void
Canvas::react(Issue issue)
{
    if (Issue::ProcNextDone == issue) {
        this->exec();
    } else if (Issue::StateChanged == issue) {
        this->respawn();
    } else if (Issue::ProcDone == issue) {
        this->paused_ = true;
    }
}


// spawn: Initialise OpenGL vertex constructs given particle positions, and draw
//        them.

void
Canvas::spawn()
{
    State& state = this->ctrl_.get_state();
    auto& xyz = this->xyz_;
    auto& rgba = this->rgba_;
    GLfloat near = this->neardef_;
    for (int i = 0; i < state.num_; ++i) {
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
    VertexBuffer* vbxyz = new VertexBuffer(xyzarray, xyz.size()
                                           * sizeof(float));
    VertexBuffer* vbrgba = new VertexBuffer(rgbaarray, rgba.size()
                                            * sizeof(float));
    VertexBuffer* vbquad = new VertexBuffer(quad, sizeof(quad));
    VertexArray* va = new VertexArray();
    va->add_buffer(0, *vbxyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
    va->add_buffer(1, *vbrgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
    va->add_buffer(2, *vbquad, VertexBufferAttribs::gen<GLfloat>(2, 2, 0));

    // instancing
    DOGL(glVertexAttribDivisor(0, 1));
    DOGL(glVertexAttribDivisor(1, 1));

    vbxyz->unbind();
    vbrgba->unbind();
    vbquad->unbind();
    va->unbind();
    this->vertex_buffer_xyz_ = vbxyz;
    this->vertex_buffer_rgba_ = vbrgba;
    this->vertex_buffer_quad_ = vbquad;
    this->vertex_array_ = va;
}


// respawn: Reinitialise OpenGL vertex constructs and redraw particles.

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


// draw: Execute the actual OpenGL draw call.

void
Canvas::draw(GLuint instances, GLuint instance_count, VertexArray* va,
             Shader* shader) const
{
    shader->bind();
    va->bind();
    DOGL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, instances,
                               instance_count));
}


// clear: Clear the OpenGL buffers (for between every iteration).

void
Canvas::clear() const
{
    DOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}


// next2d: Update OpenGL vertex constructs for the 2D render.
//         The 2D render still happens in a 3D environment.

void
Canvas::next2d()
{
    State& state = this->ctrl_.get_state();
    int num = state.num_;
    GLfloat scope = state.scope_;
    std::vector<float>& px = state.px_;
    std::vector<float>& py = state.py_;
    std::vector<unsigned int>& pn = state.pn_;
    std::vector<GLfloat>& xyz = this->xyz_;
    std::vector<GLfloat>& rgba = this->rgba_;
    VertexBuffer* vbxyz = this->vertex_buffer_xyz_;
    VertexBuffer* vbrgba = this->vertex_buffer_rgba_;
    VertexArray* va = this->vertex_array_;
    GLfloat near = this->neardef_;

    // insert updated particle positions into the buffer
    unsigned int xyzi = 0;
    unsigned int rgbai = 0;
    for (unsigned int p = 0; p < num; ++p) {
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
    vbxyz->update(xyzarray, xyz.size() * sizeof(float));
    vbrgba->update(rgbaarray, rgba.size() * sizeof(float));
    va->add_buffer(0, *vbxyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
    va->add_buffer(1, *vbrgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
}


// next3d: Update OpenGL vertex constructs for the 3D render.

void
Canvas::next3d()
{
    State &state = this->ctrl_.get_state();
    int num = state.num_;
    GLfloat scope = state.scope_;
    std::vector<float>& px = state.px_;
    std::vector<float>& py = state.py_;
    std::vector<unsigned int>& pn = state.pn_;
    std::vector<GLfloat>& xyz = this->xyz_;
    std::vector<GLfloat>& rgba = this->rgba_;
    VertexBuffer* vbxyz = this->vertex_buffer_xyz_;
    VertexBuffer* vbrgba = this->vertex_buffer_rgba_;
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
    for (unsigned int p = 0; p < num; ++p) {
        // shift particle levels (ie. represent passage of time via z & a)
        /* x */ this->shift(shift, level, px[p], 0.0f, xyz, xyzi, xyzspan);
        /* y */ this->shift(shift, level, py[p], 0.0f, xyz, xyzi, xyzspan);
        /* z */ this->shift(shift, level, near, 1.0f, xyz, xyzi, xyzspan);
        /* r */ this->shift(shift, level,
                            static_cast<GLfloat>(pn[p]) / (scope / 1.5f),
                            0.0f, rgba, rgbai, rgbaspan);
        /* g */ this->shift(shift, level,
                            static_cast<GLfloat>(pn[p]) / scope,
                            0.0f, rgba, rgbai, rgbaspan);
        /* b */ this->shift(shift, level, 0.7f, 0.0f, rgba, rgbai, rgbaspan);
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
    vbxyz->update(xyzarray, xyz.size() * sizeof(float));
    vbrgba->update(rgbaarray, rgba.size() * sizeof(float));
    va->add_buffer(0, *vbxyz, VertexBufferAttribs::gen<GLfloat>(3, 3, 0));
    va->add_buffer(1, *vbrgba, VertexBufferAttribs::gen<GLfloat>(4, 4, 0));
}


// shift: Convenience function for Next3d that does one shift of the levels.

void
Canvas::shift(bool shift, unsigned int level, GLfloat next, GLfloat inc,
              std::vector<GLfloat>& v, unsigned int& i, unsigned int span)
{
    if (!shift) {
        return;
    }
    float shifted;
    // deeper levels
    for (unsigned int l = level; l >= 1; --l) {
        shifted = v[i + ((l - 1) * span)] + inc;
        // encode alpha shift as a negative d
        if (inc < 0) {
            shifted = 0.25;
        }
        v[i + (l * span)] = shifted;
    }
    // base level
    v[i++] = next;
}


// camera_default: Apply the default MVP matrices.

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


// camera: Change the MVP matrices.

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


// camera_resize: Change the projection matrix to account for new dimensions.

void
Canvas::camera_resize(GLfloat w, GLfloat h)
{
    DOGL(glViewport(0, 0, w, h));
    this->width_ = w;
    this->height_ = h;
    this->proj_ = glm::perspective(glm::radians(50.0f), w / h, 0.1f, 100.0f);
    this->camera_set();
}

