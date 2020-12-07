#include <limits.h>

#include "gui.hh"
#include "../util/common.hh"
#include "../util/util.hh"


GuiState::GuiState(State &truth)
  : truth_(truth)
{
  this->stop_ = truth.stop_;
  this->num_ = truth.num_;
  this->width_ = truth.width_;
  this->height_ = truth.height_;
  this->alpha_ = Util::RadToDeg(truth.alpha_);
  this->beta_ = Util::RadToDeg(truth.beta_);
  this->scope_ = truth.scope_;
  this->speed_ = truth.speed_;
  this->colorscheme_ = truth.colorscheme_;
  this->preset_ = 0;
}


// Untrue: GuiState is different from State.
bool
GuiState::Untrue()
{
  return (this->stop_                  != this->truth_.stop_   ||
          this->num_                   != this->truth_.num_    ||
          this->width_                 != this->truth_.width_  ||
          this->height_                != this->truth_.height_ ||
          Util::DegToRad(this->alpha_) != this->truth_.alpha_  ||
          Util::DegToRad(this->beta_)  != this->truth_.beta_   ||
          this->scope_                 != this->truth_.scope_  ||
          this->speed_                 != this->truth_.speed_  ||
          this->colorscheme_ != this->truth_.colorscheme_);
}


// ChangeTruth: Change true system State.
bool
GuiState::ChangeTruth()
{
  StateTransport next = {this->stop_,
                         this->num_,
                         this->width_,
                         this->height_,
                         Util::DegToRad(this->alpha_),
                         Util::DegToRad(this->beta_),
                         this->scope_,
                         this->speed_,
                         this->colorscheme_};
  // truth change provokes observer (canvas) reaction
  return this->truth_.Change(next);
}


// Random: Randomise system State parameters.
void
GuiState::Random(Log &log)
{
  this->alpha_ = Util::Distribute<float>(-180.0f, 180.0f);
  this->beta_ = Util::Distribute<float>(-180.0f, 180.0f);
  this->scope_ = Util::Distribute<float>(1.0f, 96.0f);
  this->speed_ = Util::Distribute<float>(1.0f, 64.0f);
  log.Add(Attn::O, "Random: a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_)
          + ", v=" + std::to_string(this->scope_)
          + ", s=" + std::to_string(this->speed_));
  this->ChangeTruth();
}


// Preset: Apply a preset system State.
void
GuiState::Preset(Log &log)
{
  std::string preset;
  switch (this->preset_)
  {
    case 0:
      this->alpha_ = 180.0f; this->beta_ = 17.0f;
      preset = "Lifelike structures 1";
      break;
    case 1:
      this->alpha_ = 180.0f; this->beta_ = -7.0f;
      preset = "Moving structures";
      break;
    case 2:
      this->alpha_ = 180.0f; this->beta_ = -15.0f;
      preset = "Clean cow pattern";
      break;
    case 3:
      this->alpha_ = 90.0f; this->beta_ = -21.0f;
      preset = "Chaos w/ random aggr. 1";
      break;
    case 4:
      this->alpha_ = 0.0f; this->beta_ = -10.0f;
      preset = "Fingerprint pattern";
      break;
    case 5:
      this->alpha_ = 0.0f; this->beta_ = -41.0f;
      preset = "Chaos w/ random aggr. 2";
      break;
    case 6:
      this->alpha_ = 0.0f; this->beta_ = -25.0f;
      preset = "Untidy cow pattern";
      break;
    case 7:
      this->alpha_ = -180.0f; this->beta_ = -48.0f;
      preset = "Chaos w/ random aggr. 3";
      break;
    case 8:
      this->alpha_ = -180.0f; this->beta_ = 5.0f;
      preset = "Regular pattern";
      break;
    case 9:
      this->alpha_ = -159.0f; this->beta_ = 15.0f;
      preset = "Lifelike structures 2";
      break;
    case 10:
      this->alpha_ = 0.0f; this->beta_ = 1.0f;
      preset = "Stable cluster pattern";
      break;
    case 11:
      this->alpha_ = -180.0f; this->beta_ = 58.0f;
      preset = "Chaotic pattern 1";
      break;
    case 12:
      this->alpha_ = 0.0f; this->beta_ = 40.0f;
      preset = "Chaotic pattern 2";
      break;
    case 13:
      this->alpha_ = 0.0f; this->beta_ = 8.0f;
      preset = "Cells & moving cluster";
      break;
    case 14:
      this->alpha_ = 0.0f; this->beta_ = 0.0f;
      preset = "Chaotic pattern 3";
      break;
    case 15:
      this->alpha_ = 45.0f; this->beta_ = 4.0f;
      preset = "Stable rings";
      break;
  }
  log.Add(Attn::O, preset
          + ": a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_));
  this->ChangeTruth();
}


Gui::Gui(Log &log, GuiState state, Canvas &canvas, const std::string &version,
         unsigned int width, unsigned int height)
  : log_(log), state_(state), canvas_(canvas)
{
  // glfw
  GLFWwindow* view;
  if (! glfwInit())
  {
    log.Add(Attn::Egl, "glfwInit");
    return;
  }
  unsigned int gui_width = 1000;
  unsigned int gui_height = 1000;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  view = glfwCreateWindow(gui_width, gui_height, ME, NULL, NULL);
  if (! view)
  {
    log.Add(Attn::Egl, "glfwCreateWindow");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(view);
  glfwSwapInterval(1);
  glfwSetKeyCallback(view, KeyCallback);
  glfwSetMouseButtonCallback(view, MouseButtonCallback);
  glfwSetCursorPosCallback(view, MouseMoveCallback);
  glfwSetScrollCallback(view, MouseScrollCallback);
  glfwSetWindowSizeCallback(view, ResizeCallback);

  // imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  static_cast<void>(io);
  ImGui_ImplGlfw_InitForOpenGL(view, true);
  ImGui_ImplOpenGL3_Init(version.c_str());
  ImGui::StyleColorsDark();
  float font_size = 24.0f;

  this->view_ = view;
  this->font_r = io.Fonts->AddFontFromFileTTF("../opt/roboto/RobotoMono-Regular.ttf", font_size);
  this->font_b = io.Fonts->AddFontFromFileTTF("../opt/roboto/RobotoMono-Bold.ttf", font_size);
  this->font_i = io.Fonts->AddFontFromFileTTF("../opt/roboto/RobotoMono-Italic.ttf", font_size);
  this->font_z = io.Fonts->AddFontFromFileTTF("../opt/roboto/RobotoMono-BoldItalic.ttf", font_size);
  this->gui_width_ = gui_width;
  this->gui_height_ = gui_height;
  this->control_ = true;
  this->console_ = false;
  this->ago_ = glfwGetTime();
  this->frames_ = 0;
  this->fps_ = 0.0f;
  this->x_ = 0.0;
  this->y_ = 0.0;
  this->dolly_ = false;
  this->pivot_ = false;
}


Gui::~Gui()
{
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}


void
Gui::Draw()
{
  double now = glfwGetTime();
  ++this->frames_;
  if (now - this->ago_ >= 1.0)
  {
    this->fps_ = this->frames_;
    this->frames_ = 0;
    this->ago_ = now;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  this->font_width_ = ImGui::CalcTextSize(" ").x;

  this->DrawControl(this->control_);
  this->DrawConsole(this->console_);
  //ImGui::ShowDemoWindow();

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

  Log &log = this->log_;
  GuiState &state = this->state_;
  Canvas &canvas = this->canvas_;
  bool untrue = state.Untrue();
  float width = 24.0f * this->font_width_;
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  int right_alignment = width - 8;
  int button_width = 5 * this->font_width_;
  const ImU32 zero = 0;
  const ImU32 max_dim = 100000;
  const ImU32 max_num = 1000000;
  auto button_size = ImVec2(button_width, 30);
  auto color_status = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_title = ImVec4(1.0f, 0.75f, 0.25f, 1.0f);
  auto color_dim = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
  auto color_dimmer = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  std::string status;

  ImGui::SetNextWindowPos(ImVec2(view_width - width, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, static_cast<float>(view_height)),
                           ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.75f);
  if (ImGui::Begin("control", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
  {
    if (ImGui::Button("Pause")) { this->Pause(); }
    ImGui::SameLine();
    ImGui::Button("Save");
    ImGui::SameLine();
    ImGui::Button("Load");
    ImGui::SameLine();
    ImGui::SetCursorPosX(right_alignment - button_width);
    if (ImGui::Button("Quit", button_size)) { this->Quit(); }
    if (untrue)
    {
      ImGui::PushFont(this->font_z);
      status = "Parameter modified";
      ImGui::SetCursorPosX(right_alignment
                           - ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(color_status, "%s", status.c_str());
      ImGui::PopFont();
    }
    else
    {
      ImGui::PushFont(this->font_i);
      status = "Parameters in sync";
      ImGui::SetCursorPosX(right_alignment
                           - ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.f), "%s", status.c_str());
      ImGui::PopFont();
    }
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Particle");
    if (untrue)
    {
      ImGui::SameLine();
      ImGui::PushFont(this->font_z);
      status = "ENTER to apply";
      ImGui::SetCursorPosX(right_alignment
                           - ImGui::CalcTextSize(status.c_str()).x);
      ImGui::TextColored(color_status, "%s", status.c_str());
      ImGui::PopFont();
    }
    ImGui::PopFont();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("num   "); ImGui::SameLine();
    ImGui::InputScalar("n", ImGuiDataType_U32, &state.num_, &zero, &max_num);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "World");
    ImGui::PopFont();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("width "); ImGui::SameLine();
    ImGui::InputScalar("w", ImGuiDataType_U32, &state.width_, &zero, &max_dim);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("height"); ImGui::SameLine();
    ImGui::InputScalar("h", ImGuiDataType_U32, &state.height_, &zero, &max_dim);
    if (ImGui::Combo("", &state.preset_,
                     "lifelike structures 1\0"
                     "moving structures\0"
                     "clean cow pattern\0"
                     "chaos w/ random aggr. 1\0"
                     "fingerprint pattern\0"
                     "chaos w/ random aggr. 2\0"
                     "untidy cow pattern\0"
                     "chaos w/ random aggr. 3\0"
                     "regular pattern\0"
                     "lifelike structures 2\0"
                     "stable cluster pattern\0"
                     "chaotic pattern 1\0"
                     "chaotic pattern 2\0"
                     "cells & moving cluster\0"
                     "chaotic pattern 3\0"
                     "stable rings\0\0")) {
      state.Preset(log);
    }
    ImGui::SameLine();
    if (ImGui::Button("Random")) { state.Random(log); }
    ImGui::AlignTextToFramePadding();
    ImGui::Text("alpha "); ImGui::SameLine();
    ImGui::InputFloat("a", &state.alpha_, -180.0f, 180.0f, "%.2f");
    ImGui::AlignTextToFramePadding();
    ImGui::Text("beta  "); ImGui::SameLine();
    ImGui::InputFloat("b", &state.beta_, -180.0f, 180.0f, "%.2f");
    ImGui::AlignTextToFramePadding();
    ImGui::Text("scope "); ImGui::SameLine();
    ImGui::InputFloat("v", &state.scope_, 1.0f, 256.0f, "%.2f");
    ImGui::AlignTextToFramePadding();
    ImGui::Text("speed "); ImGui::SameLine();
    ImGui::InputFloat("s", &state.speed_, 1.0f, 64.0f, "%.2f");
    ImGui::Text("stop:   %u", state.stop_);
    ImGui::Text("colors: %d", state.colorscheme_);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Canvas");
    ImGui::PopFont();
    ImGui::TextColored(color_dim, "%.1f", this->fps_);
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, " fps  x:");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "%.0f", this->x_);
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, "  y:");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "%.0f", 1000 - this->y_);
    ImGui::TextColored(color_dimmer, "opencl: ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "%s",
                       canvas.proc_->cl_good_ ? "on" : "off");
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Usage");
    ImGui::PopFont();
    ImGui::TextColored(color_dimmer, "quit:    ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Escape");
    ImGui::TextColored(color_dimmer, "pause:   ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Space");
    ImGui::TextColored(color_dimmer, "camera   ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, "mouse");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "L");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, " mouse");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "R");
    ImGui::TextColored(color_dim, "          QWE   UIO");
    ImGui::TextColored(color_dimmer, "  dolly:->");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "A D");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, " .>");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "J L");
    ImGui::TextColored(color_dim, "          ZXC");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, " | ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "M,.");
    ImGui::TextColored(color_dimmer, "  pivot:------'");
    ImGui::TextColored(color_dimmer, "  zoom:  ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Minus");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dimmer, "/");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Equal");
    ImGui::TextColored(color_dimmer, "         mouse");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "wheel");
    ImGui::TextColored(color_dimmer, "  reset: ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Slash");
    ImGui::TextColored(color_dimmer, "console: ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Grave");
    ImGui::TextColored(color_dimmer, "control: ");
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
    ImGui::TextColored(color_dim, "Tab");
  }
  ImGui::End();
}


void
Gui::DrawConsole(bool draw)
{
  if (! draw)
  {
    return;
  }

  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  float width = view_width - 23.0f * this->font_width_ - 16.0f;
  float height = view_height / 2.0f;
  auto color_e = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
  auto color_ecl = ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
  auto color_egl = ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
  ImVec4 color;
  std::deque<std::pair<Attn,std::string>> &messages = this->log_.messages_;
  unsigned int count = messages.size();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.75f);
  if (ImGui::Begin("console", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
  {
    for (std::pair<Attn,std::string> &message : messages)
    {
      if      (message.first == Attn::E) { color = color_e; }
      else if (message.first == Attn::Ecl) { color = color_ecl; }
      else if (message.first == Attn::Egl) { color = color_egl; }
      if (message.first == Attn::O)
      {
        ImGui::Text("%d: %s", count, message.second.c_str());
      }
      else
      {
        ImGui::TextColored(color, "%d: %s", count, message.second.c_str());
      }
      --count;
    }
    //ImGui::SetWindowScrollY(0);
  }
  ImGui::End();
}


void
Gui::Next() const
{
  glfwSwapBuffers(this->view_);
  glfwPollEvents();
}


void
Gui::Pause()
{
  this->canvas_.Pause();
}


void
Gui::Quit()
{
  this->canvas_.HardPause();
  this->Close();
}


void
Gui::SetPointer()
{
  // for KeyCallback to access gui
  glfwSetWindowUserPointer(this->view_, this);
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
  Canvas &canvas = gui->canvas_;

  if (action == GLFW_PRESS)
  {
    if (key == GLFW_KEY_ESCAPE) { gui->Quit(); return; }
    if (key == GLFW_KEY_ENTER) { gui->state_.ChangeTruth(); return; }
    if (key == GLFW_KEY_SPACE) { gui->Pause(); return; }
    if (key == GLFW_KEY_TAB) { gui->control_ = ! gui->control_; return; }
    if (key == GLFW_KEY_GRAVE_ACCENT) { gui->console_ = ! gui->console_; return; }
    if (key == GLFW_KEY_SLASH) { canvas.CameraDefault(); return; }
  }
  float d = canvas.dollyd_;
  float z = canvas.zoomd_;
  float p = canvas.pivotd_;
  // camera translate x y
  if (key == GLFW_KEY_W) { canvas.Camera(   0,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_A) { canvas.Camera(   d,   0,0,0,0); return; }
  if (key == GLFW_KEY_X) { canvas.Camera(   0,   d,0,0,0); return; }
  if (key == GLFW_KEY_D) { canvas.Camera(-1*d,   0,0,0,0); return; }
  if (key == GLFW_KEY_Q) { canvas.Camera(   d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_E) { canvas.Camera(-1*d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_Z) { canvas.Camera(   d,   d,0,0,0); return; }
  if (key == GLFW_KEY_C) { canvas.Camera(-1*d,   d,0,0,0); return; }
  // camera translate z
  if (key == GLFW_KEY_EQUAL) { canvas.Camera(0,0,   z,0,0); return; }
  if (key == GLFW_KEY_MINUS) { canvas.Camera(0,0,-1*z,0,0); return; }
  // camera rotate
  if (key == GLFW_KEY_I) { canvas.Camera(0,0,0,   p,   0); return; }
  if (key == GLFW_KEY_J) { canvas.Camera(0,0,0,   0,   p); return; }
  if (key == GLFW_KEY_COMMA) { canvas.Camera(0,0,0,-1*p,0); return; }
  if (key == GLFW_KEY_L) { canvas.Camera(0,0,0,   0,-1*p); return; }
  if (key == GLFW_KEY_U) { canvas.Camera(0,0,0,   p,   p); return; }
  if (key == GLFW_KEY_O) { canvas.Camera(0,0,0,   p,-1*p); return; }
  if (key == GLFW_KEY_M) { canvas.Camera(0,0,0,-1*p,   p); return; }
  if (key == GLFW_KEY_PERIOD) { canvas.Camera(0,0,0,-1*p,-1*p); return; }
}


void
Gui::MouseButtonCallback(GLFWwindow* view, int button, int action, int mods)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  if (action == GLFW_PRESS)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT) { gui->pivot_ = true; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->dolly_ = true; return; }
    return;
  }
  if (action == GLFW_RELEASE)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT) { gui->pivot_ = false; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->dolly_ = false; return; }
    return;
  }
}


void
Gui::MouseMoveCallback(GLFWwindow* view, double x, double y)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
  Canvas &canvas = gui->canvas_;
  double dx = x - gui->x_;
  double dy = y - gui->y_;
  dx *= 0.00005f;
  dy *= 0.00005f;
  if (gui->dolly_) { canvas.Camera(dx,-1.0f*dy,0,0,0); return; }
  dx *= 100.0f;
  dy *= 100.0f;
  if (gui->pivot_) { canvas.Camera(0,0,0,dy,dx); return; }

  gui->x_ = x;
  gui->y_ = y;
}


void
Gui::MouseScrollCallback(GLFWwindow* view, double dx, double dy)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
  // horizontal scroll unused
  dy *= 0.1f;
  gui->canvas_.Camera(0,0,dy,0,0);
}


void
Gui::ResizeCallback(GLFWwindow* view, int w, int h)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  gui->canvas_.CameraResize(w, h);
}

