#include <limits.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include "gui.hh"
#include "../util/common.hh"
#include "../util/util.hh"


GuiState::GuiState(State &truth)
  : truth_(truth)
{
  this->distribution_ = truth.distribution_;
  this->stop_ = truth.stop_;
  this->num_ = truth.num_;
  this->width_ = truth.width_;
  this->height_ = truth.height_;
  this->alpha_ = truth.alpha_;
  this->beta_ = truth.beta_;
  this->scope_ = truth.scope_;
  this->speed_ = truth.speed_;
  this->colorscheme_ = truth.colorscheme_;
}


// ChangeTruth: Change true system State.
bool
GuiState::ChangeTruth()
{
  std::cout << this->alpha_ << std::endl;
  StateTransport next = { this->distribution_,
                          this->stop_,
                          this->num_,
                          this->width_,
                          this->height_,
                          this->alpha_,
                          this->beta_,
                          this->scope_,
                          this->speed_,
                          this->colorscheme_ };
  return this->truth_.Change(next);
}


// Untrue: GuiState is different from State.
bool
GuiState::Untrue()
{
  return (this->distribution_ != this->truth_.distribution_ ||
          this->stop_         != this->truth_.stop_         ||
          this->num_          != this->truth_.num_          ||
          this->width_        != this->truth_.width_        ||
          this->height_       != this->truth_.height_       ||
          this->alpha_        != this->truth_.alpha_        ||
          this->beta_         != this->truth_.beta_         ||
          this->scope_        != this->truth_.scope_        ||
          this->speed_        != this->truth_.speed_        ||
          this->colorscheme_  != this->truth_.colorscheme_);
}


Gui::Gui(GuiState state, Sys* sys, const std::string &version,
         unsigned int width, unsigned int height)
  : state_(state), sys_(sys)
{
  // glfw
  GLFWwindow* view;
  if (! glfwInit())
  {
    Util::Err("glfwInit");
    return;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  view = glfwCreateWindow(width, height, ME, NULL, NULL);
  if (! view)
  {
    Util::Err("glfwCreateWindow");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(view);
  glfwSwapInterval(1);
  glfwSetKeyCallback(view, KeyCallback);

  // imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  static_cast<void>(io);
  ImGui_ImplGlfw_InitForOpenGL(view, true);
  ImGui_ImplOpenGL3_Init(version.c_str());
  ImGui::StyleColorsDark();

  this->view_ = view;
  this->ago_ = glfwGetTime();
  this->frames_ = 0;
  this->fps_ = 0;
  this->control_ = true;
  this->control_width_ = 300;
}


Gui::~Gui()
{
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}


void
Gui::SetPointer()
{
  // for KeyCallback to access gui
  glfwSetWindowUserPointer(this->view_, this);
}


void
Gui::SetVisualiser(Visualiser* visualiser)
{
  this->visualiser_ = visualiser;
}


void
Gui::Close()
{
  glfwSetWindowShouldClose(this->view_, true);
}


bool
Gui::Closing() const
{
  return glfwWindowShouldClose(this->view_);
}


void
Gui::KeyCallback(GLFWwindow* view, int key, int scancode, int action,
                 int mods)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  if (action == GLFW_PRESS)
  {
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
    {
      gui->Close();
      return;
    }
    if (key == GLFW_KEY_ENTER)
    {
      if (gui->state_.ChangeTruth())
      {
        gui->visualiser_->Respawn();
      }
      return;
    }
    if (key == GLFW_KEY_C) {
      gui->control_ = ! gui->control_;
      return;
    }
  }
  if (key == GLFW_KEY_H) { gui->visualiser_->West();  return; }
  if (key == GLFW_KEY_J) { gui->visualiser_->South(); return; }
  if (key == GLFW_KEY_K) { gui->visualiser_->North(); return; }
  if (key == GLFW_KEY_L) { gui->visualiser_->East();  return; }
  if (key == GLFW_KEY_Y) { gui->visualiser_->NorthWest(); return; }
  if (key == GLFW_KEY_U) { gui->visualiser_->NorthEast(); return; }
  if (key == GLFW_KEY_B) { gui->visualiser_->SouthWest(); return; }
  if (key == GLFW_KEY_N) { gui->visualiser_->SouthEast(); return; }
  if (key == GLFW_KEY_PERIOD) { }
}


bool
Gui::IsItemJustReleased()
{
  /**
  ImGuiContext &g = *GImGui;
  bool b = false;
  if (g.ActiveIdPreviousFrame)
  {
    b = g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
  }
  return b && ! ImGui::IsItemActive();
  //*/
  return true;
}


void
Gui::Draw()
{
  //double now = glfwGetTime();
  //++this->frames_;
  //if (now - this->ago_ >= 1.0)
  //{
  //  this->fps_ = this->frames_;
  //  this->frames_ = 0;
  //  this->ago_ = now;
  //}

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  this->DrawControl(this->control_);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void
Gui::DrawControl(bool draw)
{
  if (! draw)
  {
    return;
  }

  GuiState &state = this->state_;
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  unsigned int width = this->control_width_;
  int right_alignment = width - 2 * ImGui::GetStyle().ItemSpacing.x;
  float alpha = Util::RadToDeg(state.alpha_);
  float beta = Util::RadToDeg(state.beta_);
  const ImU32 zero = 0;
  const ImU32 max_dim = 100000;
  const ImU32 max_num = 1000000;
  std::string status;

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width),
                                  static_cast<float>(view_height)),
                           ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.75f);
  if (ImGui::Begin("control", NULL, ImGuiWindowFlags_NoResize))
  {
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    if (state.Untrue())
    {
      status = "Parameters edited";
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right_alignment -
                           ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", status.c_str());
      status = "(ENTER to apply)";
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right_alignment -
                           ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", status.c_str());
    }
    else
    {
      status = "Parameters in sync";
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right_alignment -
                           ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.f), "%s", status.c_str());
      ImGui::Text("");
    }
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Particles");
    ImGui::InputScalar("num", ImGuiDataType_U32, &state.num_, &zero, &max_num);
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "World");
    ImGui::Text("fps:          %.1f", this->fps_);
    ImGui::InputScalar("width", ImGuiDataType_U32, &state.width_, &zero, &max_dim);
    ImGui::InputScalar("height", ImGuiDataType_U32, &state.height_, &zero, &max_dim);
    ImGui::InputFloat("alpha", &alpha, -M_PI, M_PI, "%.4f");
    ImGui::InputFloat("beta", &beta, -M_PI, M_PI, "%.4f");
    ImGui::InputFloat("scope", &state.scope_, 0.0f, 1000.0f, "%.4f");
    ImGui::InputFloat("speed", &state.speed_, 0.0f, 1000.0f, "%.4f");
    ImGui::Text("distribution: %u", state.distribution_);
    ImGui::Text("stop:         %u", state.stop_);
    ImGui::Text("color scheme: %d", state.colorscheme_);
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "History");
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
  }
  ImGui::End();
}


void
Gui::Next() const
{
  glfwSwapBuffers(this->view_);
  glfwPollEvents();
}
