#pragma once

#include "gl.hh"
#include "gui.hh"
#include "view.hh"
#include "../util/log.hh"


// Canvas: The graphical View.

class Gui;

class Canvas : public View, Observer
{
  public:
    Canvas(Log& log, Control& ctrl,
           unsigned int width, unsigned int height, bool hide_ctrl);
    ~Canvas() override;
    void exec() override;
    void react(Issue issue) override;
    void spawn();
    void next2d();
    void next3d();
    void shift(bool shift, unsigned int level, GLfloat next, GLfloat inc,
               std::vector<GLfloat>& v, unsigned int& i, unsigned int span);
    void camera_default();
    void camera(GLfloat dx, GLfloat dy, GLfloat dz, GLfloat dax, GLfloat day);
    void camera_resize(GLfloat w, GLfloat h);
    void respawn();
    void clear() const;
    void draw(GLuint instances, GLuint instance_count, VertexArray* va,
              Shader* shader) const;

    // three: Set 3D mode.
    inline void
    three(bool yesno)
    {
        this->three_ = yesno;
    }

    // hard_pause: Set hard pause.
    inline void
    hard_pause(bool yesno)
    {
        this->hard_paused_ = yesno;
        this->ctrl_.pause(yesno);
    }

    // pause: Toggle (soft) pause.
    inline void
    pause()
    {
        this->paused_ = !this->paused_;
        this->ctrl_.pause(this->paused_);
    }

    // quit: Turn off system processing.
    inline void
    quit()
    {
        this->ctrl_.quit();
    }

    // camera_set: Apply the current MVP matrices.
    inline void
    camera_set()
    {
        this->shader_->set_uniform_mat4f("mvp", this->proj_ * this->view_
                                         * this->model_ * this->orth_);
    }

    Gui*          gui_;
    Control&      ctrl_;
    VertexBuffer* vertex_buffer_xyz_;
    VertexBuffer* vertex_buffer_rgba_;
    VertexBuffer* vertex_buffer_quad_;
    VertexArray*  vertex_array_;
    Shader*       shader_;
    GLfloat dollyd_;   // camera position increment
    GLfloat pivotd_;   // camera pivot angle increment
    GLfloat zoomd_;    // camera zoom increment
    bool hard_paused_; // no particle movement nor user's movement input
    bool paused_;      // no particle movement but accept user movement input

  private:
    Log& log_;
    std::vector<GLfloat> xyz_;  // position vector
    std::vector<GLfloat> rgba_; // color vector
    glm::mat4 orth_;            // orthogonalisation matrix
    glm::mat4 model_;           // model-to-world matrix
    glm::mat4 view_;            // world-to-view matrix
    glm::mat4 proj_;            // perspective-projection matrix
    glm::vec3 dolly_;           // camera position vector
    GLfloat width_;             // canvas width
    GLfloat height_;            // canvas width
    GLfloat pivotax_;           // camera horizontal pivot angle
    GLfloat pivotay_;           // camera vertical pivot angle
    GLfloat pivotx_;            // camera horizontal pivot amount
    GLfloat pivoty_;            // camera vertical pivot amount
    GLfloat zoomdef_;           // camera zoom default
    GLfloat neardef_;           // model's "near" default
    bool    three_;
    unsigned int levels_;       // total number of (z-)levels
    unsigned int level_;        // current number of levels
    unsigned int shift_counts_; // number of iterations until level shift
    unsigned int shift_count_;  // current iteration until level shift
    double ago_;
};

