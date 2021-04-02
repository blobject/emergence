#include "gui.hh"
#include "../util/common.hh"

#include <regex>


GuiState::GuiState(Control& ctrl)
  : ctrl_(ctrl)
{
  State& truth    = ctrl.get_state();
  this->stop_     = ctrl.stop_;
  this->num_      = truth.num_;
  this->width_    = truth.width_;
  this->height_   = truth.height_;
  this->alpha_    = Util::rad_to_deg(truth.alpha_);
  this->beta_     = Util::rad_to_deg(truth.beta_);
  this->scope_    = truth.scope_;
  this->speed_    = truth.speed_;
  this->prad_     = truth.prad_;
  this->coloring_ = (Coloring)truth.coloring_;
  this->preset_   = 0;
}


Stative
GuiState::current() const
{
  // TODO: stop
  return {this->stop_,
          this->num_,
          this->width_,
          this->height_,
          Util::deg_to_rad(this->alpha_),
          Util::deg_to_rad(this->beta_),
          this->scope_,
          this->speed_,
          this->prad_,
          (int)this->coloring_};
}


bool
GuiState::untrue() const
{
  Stative state = this->current();
  return this->ctrl_.different(state);
}


bool
GuiState::deceive() const
{
  Stative state = this->current();
  // truth change provokes Canvas (observer) reaction
  return this->ctrl_.change(state);
}


void
GuiState::random(Log& log)
{
  this->alpha_ = Util::distribute<float>(-180.0f, 180.0f);
  this->beta_ = Util::distribute<float>(-180.0f, 180.0f);
  this->scope_ = Util::distribute<float>(1.0f, 96.0f);
  this->speed_ = Util::distribute<float>(1.0f, 32.0f);
  log.add(Attn::O,
          "Random: a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_)
          + ", v=" + std::to_string(this->scope_)
          + ", s=" + std::to_string(this->speed_));
  this->deceive();
}


void
GuiState::coloring(Log& log, Coloring scheme)
{
  std::string message;
  if      (Coloring::Normal    == scheme) { message = "normal"; }
  else if (Coloring::Cluster   == scheme) { message = "cluster-based"; }
  else if (Coloring::Density10 == scheme) { message = "density threshold 10"; }
  else if (Coloring::Density15 == scheme) { message = "density threshold 15"; }
  else if (Coloring::Density20 == scheme) { message = "density threshold 20"; }
  else if (Coloring::Density25 == scheme) { message = "density threshold 25"; }
  else if (Coloring::Density30 == scheme) { message = "density threshold 30"; }
  this->coloring_ = scheme;
  this->ctrl_.coloring(scheme);
  log.add(Attn::O, "Color scheme to " + message + ".");
  this->deceive();
}


void
GuiState::preset(Log& log)
{
  std::string message;
  int preset = this->preset_;
  if (0 == preset) {
    this->alpha_ = 180.0f; this->beta_ = 17.0f;
    message = "Lifelike structures 1";
  } else if (1 == preset) {
    this->alpha_ = 180.0f; this->beta_ = -7.0f;
    message = "Moving structures";
  } else if (2 == preset) {
    this->alpha_ = 180.0f; this->beta_ = -15.0f;
    message = "Clean cow pattern";
  } else if (3 == preset) {
    this->alpha_ = 90.0f; this->beta_ = -21.0f;
    message = "Chaos w/ random aggr. 1";
  } else if (4 == preset) {
    this->alpha_ = 0.0f; this->beta_ = -10.0f;
    message = "Fingerprint pattern";
  } else if (5 == preset) {
    this->alpha_ = 0.0f; this->beta_ = -41.0f;
    message = "Chaos w/ random aggr. 2";
  } else if (6 == preset) {
    this->alpha_ = 0.0f; this->beta_ = -25.0f;
    message = "Untidy cow pattern";
  } else if (7 == preset) {
    this->alpha_ = -180.0f; this->beta_ = -48.0f;
    message = "Chaos w/ random aggr. 3";
  } else if (8 == preset) {
    this->alpha_ = -180.0f; this->beta_ = 5.0f;
    message = "Regular pattern";
  } else if (9 == preset) {
    this->alpha_ = -159.0f; this->beta_ = 15.0f;
    message = "Lifelike structures 2";
  } else if (10 == preset) {
    this->alpha_ = 0.0f; this->beta_ = 1.0f;
    message = "Stable cluster pattern";
  } else if (11 == preset) {
    this->alpha_ = -180.0f; this->beta_ = 58.0f;
    message = "Chaotic pattern 1";
  } else if (12 == preset) {
    this->alpha_ = 0.0f; this->beta_ = 40.0f;
    message = "Chaotic pattern 2";
  } else if (13 == preset) {
    this->alpha_ = 0.0f; this->beta_ = 8.0f;
    message = "Cells & moving cluster";
  } else if (14 == preset) {
    this->alpha_ = 0.0f; this->beta_ = 0.0f;
    message = "Chaotic pattern 3";
  } else if (15 == preset) {
    this->alpha_ = 45.0f; this->beta_ = 4.0f;
    message = "Stable rings";
  }
  log.add(Attn::O,
          message
          + ": a=" + std::to_string(this->alpha_)
          + ", b=" + std::to_string(this->beta_));
  this->deceive();
}


inline bool
GuiState::save(const std::string& path)
{
  return this->ctrl_.save(path);
}


bool
GuiState::load(const std::string& path)
{
  Stative loaded = this->ctrl_.load(path);
  if (-1 == loaded.num) {
    return false;
  }
  this->stop_     = loaded.stop;
  this->num_      = loaded.num;
  this->width_    = loaded.width;
  this->height_   = loaded.height;
  this->alpha_    = Util::rad_to_deg(loaded.alpha);
  this->beta_     = Util::rad_to_deg(loaded.beta);
  this->scope_    = loaded.scope;
  this->speed_    = loaded.speed;
  this->prad_     = loaded.prad;
  this->coloring_ = (Coloring)loaded.coloring;
  return true;
}


Gui::Gui(Log& log, GuiState state, Canvas& canvas,
         unsigned int width, unsigned int height, bool two)
  : canvas_(canvas), state_(state), log_(log)
{
  // glfw
  GLFWwindow* view;
  if (!glfwInit()) {
    log.add(Attn::Egl, "glfwInit");
    return;
  }
  float dpi = (float)DPI / 100.0f;
  unsigned int gui_width = width / dpi;
  unsigned int gui_height = height / dpi;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  view = glfwCreateWindow(gui_width, gui_height, ME, NULL, NULL);
  if (!view) {
    log.add(Attn::Egl, "glfwCreateWindow");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(view);
  glfwSwapInterval(1);
  glfwSetKeyCallback(view, key_callback);
  glfwSetCursorPosCallback(view, mouse_move_callback);
  glfwSetMouseButtonCallback(view, mouse_button_callback);
  glfwSetScrollCallback(view, mouse_scroll_callback);
  glfwSetWindowSizeCallback(view, resize_callback);

  // imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  static_cast<void>(io);
  ImGui_ImplGlfw_InitForOpenGL(view, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);
  ImGui::StyleColorsDark();
  float font_size = 24.0f / dpi;

  this->view_ = view;
  this->dpi_ = dpi;
  std::string font = Util::emergence_dir() + "/../opt/fonts/LiberationMono-";
  this->font_r = io.Fonts->AddFontFromFileTTF(
    (font + "Regular.ttf").c_str(), font_size);
  this->font_b = io.Fonts->AddFontFromFileTTF(
    (font + "Bold.ttf").c_str(), font_size);
  this->font_i = io.Fonts->AddFontFromFileTTF(
    (font + "Italic.ttf").c_str(), font_size);
  this->font_z = io.Fonts->AddFontFromFileTTF(
    (font + "BoldItalic.ttf").c_str(), font_size);
  this->gui_width_ = gui_width;
  this->gui_height_ = gui_height;
  this->brief_ = true;
  this->messages_ = false;
  this->dialog_ = ' ';
  this->ago_ = glfwGetTime();
  this->frames_ = 0;
  this->fps_ = 0.0f;
  this->x_ = 0.0;
  this->y_ = 0.0;
  this->three_ = !two;
  this->dolly_ = false;
  this->pivot_ = false;
  this->cluster_radius_ = 10.0f;
  this->cluster_minpts_ = 10;
  this->density_threshold_ = 0;
}


Gui::~Gui()
{
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}


void
Gui::draw()
{
  double now = glfwGetTime();
  ++this->frames_;
  if (now - this->ago_ >= 1.0) {
    this->fps_ = this->frames_;
    this->frames_ = 0;
    this->ago_ = now;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  this->font_width_ = ImGui::CalcTextSize(" ").x;

  this->draw_brief(this->brief_);
  this->draw_messages(this->messages_);
  this->draw_config(this->dialog_);
  this->draw_save_load(this->dialog_);
  this->draw_quit(this->dialog_);
  //ImGui::ShowDemoWindow();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void
Gui::draw_brief(bool draw)
{
  if (!draw) {
    return;
  }
  GuiState& state = this->state_;
  Canvas& canvas = this->canvas_;
  int width;
  int height;
  glfwGetFramebufferSize(this->view_, &width, &height);
  auto color = ImVec4(0.75f, 0.75f, 0.75f, 0.75f);
  auto color_b = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const char* status = "running";
  if (canvas.paused_) {
    status = "paused";
  }
  ImGuiStyle& style = ImGui::GetStyle();
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.0f);
  style.WindowBorderSize = 0.0f;
  if (ImGui::Begin("brief", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%s", status); ImGui::SameLine();
    ImGui::PopFont();
    ImGui::TextColored(color, "(");
    this->backspace(2);
    ImGui::PushFont(this->font_i);
    ImGui::TextColored(color, "Space"); ImGui::SameLine();
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color, ")");
    ImGui::TextColored(color, "x"); ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", this->x_); ImGui::SameLine();
    ImGui::PopFont();
    ImGui::TextColored(color, "y"); ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", height - this->y_);
    ImGui::PopFont();
    ImGui::TextColored(color, "fps"); ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.1f", this->fps_);
    ImGui::PopFont();
    ImGui::TextColored(color, "opencl"); ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%s",
                       state.ctrl_.cl_good() ? "on" : "off");
    ImGui::PopFont();
    ImGui::PushFont(this->font_i);
    ImGui::TextColored(color, "Escape");
    ImGui::PopFont();
    this->backspace(-4);
    ImGui::TextColored(color, "for more");
  }
  ImGui::End();
  style.WindowBorderSize = 1.0f;
}


void
Gui::draw_messages(bool draw)
{
  if (!draw) {
    return;
  }
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  float width = view_width;
  float height = view_height / 2.0f;
  auto color_e = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
  auto color_ecl = ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
  auto color_egl = ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
  ImVec4 color;
  std::deque<std::pair<Attn,std::string>>& messages = this->log_.messages_;
  unsigned int count = messages.size();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.75f);
  if (ImGui::Begin("messages", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
    for (std::pair<Attn,std::string>& message : messages) {
      if      (message.first == Attn::E)   { color = color_e; }
      else if (message.first == Attn::Ecl) { color = color_ecl; }
      else if (message.first == Attn::Egl) { color = color_egl; }
      if (message.first == Attn::O) {
        ImGui::Text("%d: %s", count, message.second.c_str());
      } else {
        ImGui::TextColored(color, "%d: %s", count, message.second.c_str());
      }
      --count;
    }
  }
  ImGui::End();
}


void
Gui::draw_config(char dialog)
{
  if (dialog != 'c') {
    return;
  }
  Log& log = this->log_;
  GuiState& state = this->state_;
  Canvas& canvas = this->canvas_;
  bool untrue = state.untrue();
  float margin = 4.0f * this->font_width_;
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  float width = view_width - 2.0f * margin;
  float height = view_height - 2.0f * margin;
  auto color_status = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_title = ImVec4(1.0f, 0.75f, 0.25f, 1.0f);
  auto color_exp = ImVec4(0.25f, 1.00f, 0.75f, 1.0f);
  auto color_dim = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
  auto color_dimmer = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  const ImS64 negone = -1;
  const ImU32 zero = 0;
  const ImU32 max_dim = 100000;
  const ImU32 max_num = 1000000;
  const ImS64 max_stop = 2000000000;
  std::string pause = "Pause";
  if (canvas.paused_) {
    pause = "Resume";
  }
  std::string status;

  ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.75f);
  if (ImGui::Begin("config", NULL,
                   ImGuiWindowFlags_HorizontalScrollbar |
                   ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoResize)) {
    if (ImGui::Button(pause.c_str())) {
      this->pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      this->dialog_ = 's';
      if (!canvas_.paused_) {
        this->pause();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
      this->dialog_ = 'l';
      if (!canvas_.paused_) {
        this->pause();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit")) {
      this->dialog_ = 'q';
      if (!canvas_.paused_) {
        this->pause();
      }
    }
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Graphics");
    ImGui::PopFont();
    if (ImGui::Checkbox("3d", &this->three_)) {
      canvas.three(this->three_);
    }
    ImGui::Text("colors         %d", (int)state.coloring_);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("particle size"); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputFloat("r", &state.prad_, 0.5f, 64.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    if (untrue) {
      ImGui::PushFont(this->font_z);
      status = "Parameter modified (Enter to apply)";
      ImGui::TextColored(color_status, "%s", status.c_str());
      ImGui::PopFont();
    } else {
      ImGui::PushFont(this->font_i);
      status = "Parameters in sync";
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.f), "%s",
                         status.c_str());
      ImGui::PopFont();
    }
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Habitat");
    ImGui::PopFont();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("num   "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputScalar("n", ImGuiDataType_U32, &state.num_, &zero, &max_num);
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("width "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputScalar("w", ImGuiDataType_U32, &state.width_, &zero, &max_dim);
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("height"); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputScalar("h", ImGuiDataType_U32, &state.height_, &zero,
                       &max_dim);
    ImGui::PopItemWidth();
    if (ImGui::Button("Random pattern")) {
      state.random(log);
    }
    this->auto_width(2);
    ImGui::SameLine();
    if (ImGui::Combo("p", &state.preset_,
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
      state.preset(log);
    }
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("alpha "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputFloat("a", &state.alpha_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("beta  "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputFloat("b", &state.beta_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("scope "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputFloat("v", &state.scope_, 1.0f, 256.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("speed "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputFloat("s", &state.speed_, 1.0f, 64.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("stop  "); ImGui::SameLine();
    this->auto_width(2);
    ImGui::InputScalar("!", ImGuiDataType_S64, &state.ctrl_.stop_,
                       &negone, &max_stop);
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Analysis");
    ImGui::PopFont();
    if (ImGui::Button("Reset experiment module & colors")) {
      state.ctrl_.reset_exp();
      this->message_exp_cluster_ = "";
      this->message_exp_inject_ = "";
      this->message_exp_density_ = "";
      state.coloring(log, Coloring::Normal);
    }
    if (ImGui::Button("Half color")) {
      state.coloring(log, Coloring::Half);
    }
    if (ImGui::Button("Count clusters")) {
      //this->message_exp_cluster_ = state.ctrl_.cluster();
      this->message_exp_cluster_ = state.ctrl_.cluster2(this->cluster_radius_,
                                                this->cluster_minpts_);
      state.coloring(log, Coloring::Cluster);
    }
    this->auto_width(4);
    ImGui::SameLine();
    ImGui::InputFloat("cr", &this->cluster_radius_, 1.0f, 512.0f, "%.3f");
    ImGui::SameLine();
    ImGui::InputScalar("cn", ImGuiDataType_U32, &this->cluster_minpts_, &zero, &max_num);
    ImGui::PopItemWidth();
    if (!this->message_exp_cluster_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_cluster_.c_str());
    }
    if (ImGui::Button("Inject clusters")) {
      this->message_exp_inject_ = state.ctrl_.inject();
    }
    if (!this->message_exp_inject_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_inject_.c_str());
    }
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Density map "); ImGui::SameLine();
    this->auto_width(2);
    if (ImGui::Combo("d", &this->density_threshold_,
                     "none\0"
                     "n = 10\0"
                     "n = 15\0"
                     "n = 20\0"
                     "n = 25\0"
                     "n = 30\0\0")) {
      Coloring scheme = Coloring::Normal;
      int threshold = this->density_threshold_;
      if (0 == threshold) {
        scheme = Coloring::Normal;
        this->message_exp_density_ = "";
      } else if (1 == threshold) {
        scheme = Coloring::Density10;
        this->message_exp_density_ = "density threshold set to 10";
      } else if (2 == threshold) {
        scheme = Coloring::Density15;
        this->message_exp_density_ = "density threshold set to 15";
      } else if (3 == threshold) {
        scheme = Coloring::Density20;
        this->message_exp_density_ = "density threshold set to 20";
      } else if (4 == threshold) {
        scheme = Coloring::Density25;
        this->message_exp_density_ = "density threshold set to 25";
      } else if (5 == threshold) {
        scheme = Coloring::Density30;
        this->message_exp_density_ = "density threshold set to 30";
      }
      state.coloring(log, scheme);
    }
    ImGui::PopItemWidth();
    if (!this->message_exp_density_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_density_.c_str());
    }
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Keyboard shortcuts");
    ImGui::PopFont();
    ImGui::TextColored(color_dimmer, "pause/resume:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "Space");
    ImGui::TextColored(color_dimmer, "quit: Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "Q");
    this->backspace();
    ImGui::TextColored(color_dimmer, "/Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "C");
    ImGui::TextColored(color_dimmer, "save: Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "S");
    ImGui::TextColored(color_dimmer, "load: Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "L");
    ImGui::TextColored(color_dimmer, "show messages:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "Grave(`)");
    ImGui::TextColored(color_dimmer, "show brief:   "); ImGui::SameLine();
    ImGui::TextColored(color_dim, "S");
    ImGui::TextColored(color_dim, "               QWE          UIO");
    ImGui::TextColored(color_dimmer, "camera  dolly:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "A D"); ImGui::SameLine();
    ImGui::TextColored(color_dimmer, " pivot:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "J L");
    ImGui::TextColored(color_dim, "               ZXC          M,.");
    ImGui::TextColored(color_dimmer, "            mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "L"); ImGui::SameLine();
    ImGui::TextColored(color_dimmer, "     mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "R");
    ImGui::TextColored(color_dimmer, "         zoom:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "Minus(-)");
    this->backspace();
    ImGui::TextColored(color_dimmer, "/");
    this->backspace();
    ImGui::TextColored(color_dim, "Equal(=)");
    this->backspace();
    ImGui::TextColored(color_dimmer, "/mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "wheel");
    ImGui::TextColored(color_dimmer, "        reset:"); ImGui::SameLine();
    ImGui::TextColored(color_dim, "Slash(/)");
  }
  ImGui::End();
}


void
Gui::draw_save_load(char dialog)
{
  if (dialog != 's' && dialog != 'l') {
    return;
  }
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  int w = std::max((int)(400.0f / this->dpi_), (int)(view_width * 0.75f));
  int h = std::max((int)(250.0f / this->dpi_), (int)(view_height * 0.3f));
  std::string title = "Save state to where?";
  std::string button = "SAVE";
  bool (GuiState::*func)(const std::string&) = &GuiState::save;
  if (dialog == 'l') {
    title = "Load state from where?";
    button = "LOAD";
    func = &GuiState::load;
  }
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  static char bad_input = ' ';
  static bool bad_save = false;
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((view_width - w) / 2, (view_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin(title.c_str(), NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    ImGui::InputTextWithHint("", "/path/to/file", path,
                             IM_ARRAYSIZE(path));
    ImGui::PopItemWidth();
    if (bad_save) {
      ImGui::TextColored(color_bad, "  Load/Save failed!");
    } else if (bad_input == 'e') {
      ImGui::TextColored(color_bad, "  Path left empty");
    } else if (bad_input == 'b') {
      ImGui::TextColored(color_bad, "%s", allowed.c_str());
    } else {
      ImGui::Text("");
    }
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushFont(this->font_b);
    if (ImGui::Button(button.c_str(),
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 10,
                             0))) {
      if (std::string(path).empty()) {
        bad_input = 'e';
      } else if (!std::regex_match(path, std::regex(allowed))) {
        bad_input = 'b';
      } else {
        bad_input = ' ';
        bad_save = !(this->state_.*func)(path);
        if (!bad_save) {
          this->dialog_ = ' ';
          if (!canvas_.paused_) {
            this->pause();
          }
        }
      }
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x
                                       - 20, 0))) {
      bad_input = ' ';
      bad_save = false;
      this->dialog_ = ' ';
      this->pause();
    }
  }
  ImGui::End();
}


void
Gui::draw_quit(char dialog)
{
  if (dialog != 'q') {
    return;
  }
  int view_width;
  int view_height;
  glfwGetFramebufferSize(this->view_, &view_width, &view_height);
  int w = std::max((int)(420.0f / this->dpi_), (int)(view_width * 0.75f));
  int h = std::max((int)(460.0f / this->dpi_), (int)(view_width * 0.5f));
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  static char bad_input = ' ';
  static bool bad_save = false;
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((view_width - w) / 2, (view_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin("Save before quitting?", NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    ImGui::InputTextWithHint("", "/path/to/file", path, IM_ARRAYSIZE(path));
    ImGui::PopItemWidth();
    if (bad_save) {
      ImGui::TextColored(color_bad, "  Save failed!");
    } else if (bad_input == 'e') {
      ImGui::TextColored(color_bad, "  Path left empty");
    } else if (bad_input == 'b') {
      ImGui::TextColored(color_bad, "%s", allowed.c_str());
    } else {
      ImGui::Text("");
    }
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
    ImGui::PushFont(this->font_b);
    if (ImGui::Button("Save and QUIT",
                      ImVec2(ImGui::GetContentRegionAvail().x - 40, 0))) {
      if (std::string(path).empty()) {
        bad_input = 'e';
      } else if (!std::regex_match(path, std::regex(allowed))) {
        bad_input = 'b';
      } else {
        bad_save = !this->state_.save(path);
        if (!bad_save) {
          this->close();
        }
      }
    }
      ImGui::Text("");
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
      if (ImGui::Button("QUIT without saving",
                        ImVec2(ImGui::GetContentRegionAvail().x - 40, 0))) {
        this->close();
      }
      ImGui::PopFont();
      ImGui::Text("");
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
      if (ImGui::Button("Cancel",
                        ImVec2(ImGui::GetContentRegionAvail().x - 40, 0))) {
        bad_input = ' ';
        bad_save = false;
        this->dialog_ = ' ';
        this->pause();
      }
      ImGui::Text("");
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                         " Ctrl+Q to QUIT without saving");
  }
  ImGui::End();
}


void
Gui::auto_width(int factor)
{
  float width = std::max(ImGui::GetContentRegionAvail().x / (float)factor, 400.0f / (float)factor);
  ImGui::PushItemWidth(width);
}


void
Gui::next() const
{
  glfwSwapBuffers(this->view_);
  glfwPollEvents();
}


void
Gui::pause()
{
  this->canvas_.pause();
}


void
Gui::set_pointer()
{
  // for KeyCallback to access gui
  glfwSetWindowUserPointer(this->view_, this);
}


void
Gui::close()
{
  glfwSetWindowShouldClose(this->view_, true);
  this->canvas_.quit();
}


bool
Gui::closing() const
{
  return glfwWindowShouldClose(this->view_);
}


void
Gui::key_callback(GLFWwindow* view, int key, int /* scancode */, int action,
                  int mods)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
  Canvas& canvas = gui->canvas_;

  if (action == GLFW_PRESS) {
    if (mods & GLFW_MOD_CONTROL) {
      if ('q' == gui->dialog_) {
        if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
          gui->close();
          return;
        }
      }
      if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
        gui->dialog_ = 'q';
        if (!gui->canvas_.paused_) {
          gui->pause();
        }
        return;
      }
      if (key == GLFW_KEY_L) {
        gui->dialog_ = 'l';
        if (!gui->canvas_.paused_) {
          gui->pause();
        }
        return;
      }
      if (key == GLFW_KEY_S) {
        gui->dialog_ = 's';
        if (!gui->canvas_.paused_) {
          gui->pause();
        }
        return;
      }
    }
    if (key == GLFW_KEY_ESCAPE) {
      if (' ' == gui->dialog_) {
        gui->dialog_ = 'c';
        return;
      }
      gui->dialog_ = ' ';
      return;
    }
    if (' ' != gui->dialog_ && 'c' != gui->dialog_) {
      return;
    }
    if (key == GLFW_KEY_ENTER) { gui->state_.deceive(); return; }
    if (key == GLFW_KEY_SPACE) { gui->pause(); return; }
    if (key == GLFW_KEY_GRAVE_ACCENT) {
      gui->messages_ = !gui->messages_;
      return;
    }
    if (key == GLFW_KEY_SLASH) { canvas.camera_default(); return; }
    if (key == GLFW_KEY_S) { gui->brief_ = !gui->brief_; return; }
  }

  if (' ' != gui->dialog_) {
    return;
  }

  if (action == GLFW_RELEASE) {
    return;
  }

  float d = canvas.dollyd_;
  float z = canvas.zoomd_;
  float p = canvas.pivotd_;
  // camera translate x y
  if (key == GLFW_KEY_W) { canvas.camera(   0,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_A) { canvas.camera(   d,   0,0,0,0); return; }
  if (key == GLFW_KEY_X) { canvas.camera(   0,   d,0,0,0); return; }
  if (key == GLFW_KEY_D) { canvas.camera(-1*d,   0,0,0,0); return; }
  if (key == GLFW_KEY_Q) { canvas.camera(   d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_E) { canvas.camera(-1*d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_Z) { canvas.camera(   d,   d,0,0,0); return; }
  if (key == GLFW_KEY_C) { canvas.camera(-1*d,   d,0,0,0); return; }
  // camera translate z
  if (key == GLFW_KEY_EQUAL) { canvas.camera(0,0,   z,0,0); return; }
  if (key == GLFW_KEY_MINUS) { canvas.camera(0,0,-1*z,0,0); return; }
  // camera rotate
  if (key == GLFW_KEY_I) { canvas.camera(0,0,0,   p,   0); return; }
  if (key == GLFW_KEY_J) { canvas.camera(0,0,0,   0,   p); return; }
  if (key == GLFW_KEY_COMMA) { canvas.camera(0,0,0,-1*p,0); return; }
  if (key == GLFW_KEY_L) { canvas.camera(0,0,0,   0,-1*p); return; }
  if (key == GLFW_KEY_U) { canvas.camera(0,0,0,   p,   p); return; }
  if (key == GLFW_KEY_O) { canvas.camera(0,0,0,   p,-1*p); return; }
  if (key == GLFW_KEY_M) { canvas.camera(0,0,0,-1*p,   p); return; }
  if (key == GLFW_KEY_PERIOD) { canvas.camera(0,0,0,-1*p,-1*p); return; }
}


void
Gui::mouse_move_callback(GLFWwindow* view, double x, double y)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  if (' ' != gui->dialog_ || gui->messages_) {
    return;
  }

  Canvas& canvas = gui->canvas_;
  double dx = x - gui->x_;
  double dy = y - gui->y_;
  dx *= 0.00005f;
  dy *= 0.00005f;
  if (gui->dolly_) { canvas.camera(dx,-1.0f*dy,0,0,0); return; }
  dx *= 100.0f;
  dy *= 100.0f;
  if (gui->pivot_) { canvas.camera(0,0,0,dy,dx); return; }
  gui->x_ = x;
  gui->y_ = y;
}


void
Gui::mouse_button_callback(GLFWwindow* view, int button, int action,
                           int /* mods */)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  if (' ' != gui->dialog_ || gui->messages_) {
    return;
  }

  if (action == GLFW_PRESS) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) { gui->pivot_ = true; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->dolly_ = true; return; }
    return;
  }
  if (action == GLFW_RELEASE) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) { gui->pivot_ = false; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->dolly_ = false; return; }
    return;
  }
}


void
Gui::mouse_scroll_callback(GLFWwindow* view, double /* dx */, double dy)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

  if (' ' != gui->dialog_ || gui->messages_) {
    return;
  }

  dy *= 0.1f;
  gui->canvas_.camera(0,0,dy,0,0);
}


void
Gui::resize_callback(GLFWwindow* view, int w, int h)
{
  Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
  gui->canvas_.camera_resize(w, h);
}

