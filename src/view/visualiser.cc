// allow version 330 core shader syntax (NOTE: may be unnecessary)
#define MESA_GL_VERSION_OVERRIDE 3.3
#define MESA_GLSL_VERSION_OVERRIDE 330

#include <GL/glew.h>

#include "visualiser.hh"
#include "scene.hh"
#include "../util/util.hh"


Visualiser::Visualiser(Processor* processor, bool hide_ctrl)
  : processor_(processor)
{
  State &state = processor->get_state();
  this->width_ = state.width_;
  this->height_ = state.height_;
  this->gui_ = new Gui("#version 330 core", state.width_, state.height_);
  if (nullptr == this->gui_->window_)
  {
    return; // TODO: handle error
  }
	glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit())
  {
    Util::Err("glewInit");
  }
}


void
Visualiser::Exec()
{
  State &state = this->processor_->get_state();
  Scene scene;

  // processing
  ProcessOut data = this->processor_->All();
  unsigned int num = data.num;
  VertexArray &va = *(data.vertex_array);
  Shader &shader = *(data.shader);

  while (! this->gui_->Closing())
  {
    // pre
    this->gui_->HandleInput();
    scene.Clear();

    // render
    //va.Bind(); // optional bind
    scene.Draw(GL_FLOAT, num, va, shader);
    //va->Unbind(); // optional unbind
    this->gui_->Draw(state);

    // post
    this->gui_->Next();

    // process next
    //ProcessOut next = this->processor_->Right();
    //num = next.num;
    //va = *(next.vertex_array);
  }
}

