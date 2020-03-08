#include <GL/glew.h>

#include "visualiser.hh"
#include "drawer.hh"
#include "../util/util.hh"


Visualiser::Visualiser(Processor &processor, bool hide_ctrl)
  : processor_(processor)
  , width_(processor.state_.width_)
  , height_(processor.state_.height_)
  , gui_(Gui("#version 330 core",
             processor.state_.width_,
             processor.state_.height_))
{
  if (nullptr == this->gui_.window_)
  {
    return;
  }
	glewExperimental = true; // for core profile
  if (GLEW_OK != glewInit())
  {
    Util::Err("glewInit");
  }
  // (NOTE: seems unnecessary) allow version 330 core shader syntax
  //putenv((char *) "MESA_GL_VERSION_OVERRIDE=3.3");
  //putenv((char *) "MESA_GLSL_VERSION_OVERRIDE=330");
}


void
Visualiser::Exec()
{
  GLFWwindow* window = this->gui_.window_;
  Drawer drawer;

  // gl process
  ProcessOut processed = this->processor_.Process();
  unsigned int num = processed.Num;
  VertexArray &va = *(processed.VertexArray);
  Shader &shader = *(processed.Shader);

  while (! this->gui_.Closing())
  {
    this->gui_.HandleInput();
    drawer.Clear();

    // gl draw
    va.Bind();
    drawer.Draw(4, num, va, shader);
    //va->Unbind(); // optional unbind

    this->gui_.Draw(this->processor_.state_);
    this->gui_.Next();
  }
  delete processed.Shader;
  delete processed.VertexArray;
}

