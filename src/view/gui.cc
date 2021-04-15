#include "gui.hh"
#include <iomanip>
#include <iterator>
#include <regex>


Gui::Gui(Log& log, UiState& uistate, Canvas& canvas, GLFWwindow* window,
         float scale, bool three)
  : canvas_(canvas), log_(log), uistate_(uistate), window_(window),
    scale_(scale), three_(three)
{
  // more glfw
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_move_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, mouse_scroll_callback);

  // imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  static_cast<void>(io);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);
  ImGui::StyleColorsDark();

  float font_size = 28.0f / scale;
  this->cwd_ = Util::emergence_dir() + "/";
  std::string font = this->cwd_ + "../opt/fonts/LiberationMono-";
  this->font_r = io.Fonts->AddFontFromFileTTF(
    (font + "Regular.ttf").c_str(), font_size);
  this->font_b = io.Fonts->AddFontFromFileTTF(
    (font + "Bold.ttf").c_str(), font_size);
  this->font_i = io.Fonts->AddFontFromFileTTF(
    (font + "Italic.ttf").c_str(), font_size);
  this->font_z = io.Fonts->AddFontFromFileTTF(
    (font + "BoldItalic.ttf").c_str(), font_size);
  this->brief_ = true;
  this->messages_ = false;
  this->box_ = Box::None;
  this->box_opacity_ = 0.8f;
  this->input_focus_ = false;
  this->ago_ = glfwGetTime();
  this->frames_ = 0;
  this->fps_ = 0.0f;
  this->x_ = 0.0;
  this->y_ = 0.0;
  this->dolly_ = false;
  this->pivot_ = false;
  this->capturing_ = false;
  this->bad_capture_ = false;
  this->coloring_ = 0;
  this->cluster_radius_ = 0.5f * uistate.scope_;
  this->cluster_minpts_ = 14; // avg size of premature spore (Schmickl et al.)
  this->inject_sprite_ = 4; // triangle cell
  this->inject_dpe_ = static_cast<float>(uistate.num_)
                      / uistate.width_ / uistate.height_;
  this->inspect_particle_ = -1;
  this->inspect_cluster_ = -1;
  this->inspect_cluster_particle_ = -1;
  this->message_exp_inspect_default_ = "\n\n(Select an item on the left)";
  this->message_exp_inspect_ = this->message_exp_inspect_default_;
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

  if (this->capturing_) {
    // TODO: not capturing correctly (tearing, dialog box showing)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    this->capturing_ = false;
    int w;
    int h;
    glfwGetFramebufferSize(this->window_, &w, &h);
    if (!Image::out(this->capture_path_, w, h)) {
      this->bad_capture_ = true;
      this->box_ = Box::Capture;
      return;
    }
    this->box_ = Box::Captured;
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  this->font_width_ = ImGui::CalcTextSize(" ").x;
  this->draw_brief(this->brief_);
  this->draw_messages(this->messages_);
  this->draw_config(this->box_);
  this->draw_capture(this->box_);
  this->draw_captured(this->box_);
  this->draw_save_load(this->box_);
  this->draw_quit(this->box_);
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
  Control& ctrl = this->uistate_.ctrl_;
  int width;
  int height;
  glfwGetFramebufferSize(this->window_, &width, &height);
  auto color = ImVec4(0.75f, 0.75f, 0.75f, 0.75f);
  auto color_b = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const char* status = "running";
  if (ctrl.paused_) {
    status = "paused";
  }
  ImGuiStyle& style = ImGui::GetStyle();
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.0f);
  style.WindowBorderSize = 0.0f;
  if (ImGui::Begin("brief", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {

    // paused
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%s", status);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(color, "(");
    this->backspace();
    ImGui::PushFont(this->font_i);
    ImGui::TextColored(color, "Space");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color, ")");

    // tick
    ImGui::TextColored(color, "tick");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%lu", ctrl.tick_);
    ImGui::PopFont();
    this->backspace(-1);
    ImGui::TextColored(color, "/%lld", ctrl.start_);

    // fps
    ImGui::TextColored(color, "fps");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.1f", this->fps_);
    ImGui::PopFont();

    // mouse
    ImGui::TextColored(color, "x");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", this->x_);
    ImGui::PopFont();
    this->backspace(-1);
    ImGui::TextColored(color, "/%d, y", width);
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", height - this->y_);
    ImGui::PopFont();
    this->backspace(-1);
    ImGui::TextColored(color, "/%d", height);

    // opencl
    ImGui::TextColored(color, "opencl");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%s", ctrl.cl_good() ? "on" : "off");
    ImGui::PopFont();

    // more
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
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  float width = window_width;
  float height = window_height / 2.0f;
  auto color_dim = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  auto color_e = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
  auto color_ecl = ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
  auto color_egl = ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
  ImVec4 color;
  std::deque<std::pair<Attn,std::string>>& messages = this->log_.messages_;
  unsigned int count = messages.size();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(this->box_opacity_);
  if (ImGui::Begin("messages", NULL,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
    for (std::pair<Attn,std::string>& message : messages) {
      if      (message.first == Attn::E)   { color = color_e; }
      else if (message.first == Attn::Ecl) { color = color_ecl; }
      else if (message.first == Attn::Egl) { color = color_egl; }
      if (message.first == Attn::O) {
        ImGui::TextColored(color_dim, "%d:", count);
        ImGui::SameLine();
        ImGui::Text("%s", message.second.c_str());
      } else {
        ImGui::TextColored(color, "%d: %s", count, message.second.c_str());
      }
      --count;
    }
  }
  ImGui::End();
}


void
Gui::draw_config(Box box)
{
  if (Box::Config != box) {
    return;
  }
  Log& log = this->log_;
  UiState& uistate = this->uistate_;
  Control& ctrl = uistate.ctrl_;
  Canvas& canvas = this->canvas_;
  Exp& exp = ctrl.get_exp();
  State& state = ctrl.get_state();
  int untrue = uistate.untrue();
  bool paused = ctrl.paused_;
  float margin = 4.0f * this->font_width_;
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  float width = window_width - 2.0f * margin;
  float height = window_height - 2.0f * margin;
  float font_width = this->font_width_;
  float inspect_width =
    std::min(7 * font_width,
             (3 + std::to_string(state.num_).size()) * font_width);
  float inspect_height = 160.0f;
  auto color_status = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_title = ImVec4(1.0f, 0.75f, 0.25f, 1.0f);
  auto color_exp = ImVec4(0.25f, 1.0f, 0.75f, 1.0f);
  auto color_dim = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
  auto color_dimmer = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  const ImS64 negone = -1;
  const ImU32 zero = 0;
  const ImU32 max_dim = 100000;
  const ImU32 max_num = 1000000;
  const ImS64 max_stop = 2000000000;
  std::string pause = "Pause";
  if (paused) {
    pause = "Resume";
  }
  std::string status;

  ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(this->box_opacity_);
  if (ImGui::Begin("config", NULL,
                   ImGuiWindowFlags_HorizontalScrollbar |
                   ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoResize)) {

    // top buttons ////////////////////////////////////////////////////////////

    if (ImGui::Button(pause.c_str())) {
      ctrl.pause(!paused);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      this->box_ = Box::Save;
      this->input_focus_ = true;
      ctrl.pause(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
      this->box_ = Box::Load;
      ctrl.pause(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Take picture")) {
      this->box_ = Box::Capture;
      this->input_focus_ = true;
      ctrl.pause(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit")) {
      this->box_ = Box::Quit;
      ctrl.pause(true);
    }

    // truth //////////////////////////////////////////////////////////////////

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    if (untrue) {
      ImGui::PushFont(this->font_z);
      status = "Parameter modified (Enter to apply";
      if (0 > untrue) {
        status += " & respawn";
      }
      status += ")";
      ImGui::TextColored(color_status, "%s", status.c_str());
      ImGui::PopFont();
    } else {
      ImGui::PushFont(this->font_i);
      status = "Parameters in sync";
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.f), "%s",
                         status.c_str());
      ImGui::PopFont();
    }

    // graphics ///////////////////////////////////////////////////////////////

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Graphics");
    ImGui::PopFont();

    // box opacity
    ImGui::AlignTextToFramePadding();
    ImGui::Text("box opacity");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::SliderFloat("o", &this->box_opacity_, 0.0f, 1.0f);
    ImGui::PopItemWidth();

    // 3d
    ImGui::AlignTextToFramePadding();
    ImGui::Text("space ");
    ImGui::SameLine();
    if (ImGui::Checkbox("3D", &this->three_)) {
      canvas.three(this->three_);
    }

    // particle radius
    ImGui::AlignTextToFramePadding();
    ImGui::Text("p. rad"); ImGui::SameLine();
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("r", &uistate.prad_, 0.5f, 64.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));

    // habitat ////////////////////////////////////////////////////////////////

    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Habitat");
    ImGui::PopFont();

    // reset truth
    if (ImGui::Button("Default habitat & particle radius")) {
      uistate.prad_ = 1.0f;
      uistate.num_ = 5000;
      uistate.width_ = 250;
      uistate.height_ = 250;
      uistate.alpha_ = 180.0f;
      uistate.beta_ = 17.0f;
      uistate.scope_ = 5.0f;
      uistate.ascope_ = 1.3f;
      uistate.speed_ = 0.67f;
      log.add(Attn::O, "Habitat reset.");
    }

    // stop
    ImGui::AlignTextToFramePadding();
    ImGui::Text("stop  ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("!", ImGuiDataType_S64, &uistate.stop_,
                       &negone, &max_stop);
    ImGui::PopItemWidth();

    // num
    ImGui::AlignTextToFramePadding();
    ImGui::Text("num   ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("n", ImGuiDataType_U32, &uistate.num_, &zero, &max_num);
    ImGui::PopItemWidth();

    // width
    ImGui::AlignTextToFramePadding();
    ImGui::Text("width ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("w", ImGuiDataType_U32, &uistate.width_, &zero,
                       &max_dim);
    ImGui::PopItemWidth();

    // height
    ImGui::AlignTextToFramePadding();
    ImGui::Text("height");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("h", ImGuiDataType_U32, &uistate.height_, &zero,
                       &max_dim);
    ImGui::PopItemWidth();

    // alpha
    ImGui::AlignTextToFramePadding();
    ImGui::Text("alpha ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("a", &uistate.alpha_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();

    // beta
    ImGui::AlignTextToFramePadding();
    ImGui::Text("beta  ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("b", &uistate.beta_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();

    // scope
    ImGui::AlignTextToFramePadding();
    ImGui::Text("scope ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("v", &uistate.scope_, 1.0f, 256.0f, "%.3f");
    ImGui::PopItemWidth();

    // TODO: ascope

    // speed
    ImGui::AlignTextToFramePadding();
    ImGui::Text("speed ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("s", &uistate.speed_, 1.0f, 64.0f, "%.3f");
    ImGui::PopItemWidth();

    // preset alpha & beta
    if (ImGui::Button("Random pattern")) {
      this->message_set_ = uistate.random();
    }
    ImGui::SameLine();
    this->auto_width(width);
    if (ImGui::Combo("p", &uistate.pattern_,
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
      this->message_set_ = uistate.pattern();
    }
    ImGui::PopItemWidth();

    // analysis ///////////////////////////////////////////////////////////////

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Analysis");
    ImGui::PopFont();

    // reset analysis
    if (ImGui::Button("Reset experiment module & colors")) {
      ctrl.reset_exp();
      this->message_exp_color_ = "";
      this->message_exp_cluster_ = "";
      this->message_exp_inject_ = "";
      this->message_exp_inspect_ = this->message_exp_inspect_default_;
      this->inspect_particle_ = -1;
      this->inspect_cluster_ = -1;
      this->inspect_cluster_particle_ = -1;
      log.add(Attn::O, "Experiment module & colors reset.");
      uistate.coloring_ = Coloring::Original;
      uistate.deceive();
    }

    // coloring
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Coloring ");
    ImGui::SameLine();
    this->auto_width(width);
    if (ImGui::Combo("d", &this->coloring_,
                     "original\0"
                     "dynamic\0"
                     "density 10\0"
                     "density 15\0"
                     "density 20\0"
                     "density 25\0"
                     "density 30\0"
                     "density 35\0"
                     "density 40\0\0")) {
      this->message_exp_cluster_ = "";
      this->message_exp_inject_ = "";
      this->message_exp_inspect_ = this->message_exp_inspect_default_;
      this->inspect_particle_ = -1;
      this->inspect_cluster_ = -1;
      this->inspect_cluster_particle_ = -1;
      Coloring scheme = Coloring::Original;
      int choice = this->coloring_;
      if      (0 == choice) { scheme = Coloring::Original; }
      else if (1 == choice) { scheme = Coloring::Dynamic; }
      else if (2 == choice) { scheme = Coloring::Density10; }
      else if (3 == choice) { scheme = Coloring::Density15; }
      else if (4 == choice) { scheme = Coloring::Density20; }
      else if (5 == choice) { scheme = Coloring::Density25; }
      else if (6 == choice) { scheme = Coloring::Density30; }
      else if (7 == choice) { scheme = Coloring::Density35; }
      else if (8 == choice) { scheme = Coloring::Density40; }
      this->message_exp_color_ = ctrl.color(scheme);
      log.add(Attn::O, this->message_exp_color_);
      uistate.coloring_ = scheme;
      uistate.deceive();
    }
    ImGui::PopItemWidth();
    if (!this->message_exp_color_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_color_.c_str());
    }

    // clustering
    if (ImGui::Button("Detect clusters")) {
      this->message_exp_color_ = "";
      this->message_exp_inject_ = "";
      this->message_exp_inspect_ = this->message_exp_inspect_default_;
      this->inspect_particle_ = -1;
      this->inspect_cluster_ = -1;
      this->inspect_cluster_particle_ = -1;
      this->message_exp_cluster_ =
        ctrl.cluster(this->cluster_radius_, this->cluster_minpts_);
      log.add(Attn::O, this->message_exp_cluster_);
      ctrl.color(Coloring::Cluster);
      uistate.coloring_ = Coloring::Cluster;
      uistate.deceive();
    }
    this->auto_width(width, 3);
    ImGui::SameLine();
    ImGui::Text("radius");
    ImGui::SameLine();
    ImGui::InputFloat(";", &this->cluster_radius_, 1.0f, 10000.0f, "%.3f");
    ImGui::SameLine();
    ImGui::Text("minpts");
    ImGui::SameLine();
    ImGui::InputScalar(":", ImGuiDataType_U32, &this->cluster_minpts_, &zero, &max_num);
    ImGui::PopItemWidth();
    if (!this->message_exp_cluster_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_cluster_.c_str());
    }

    // injection
    if (ImGui::Button("Inject clusters")) {
      this->message_exp_color_ = "";
      this->message_exp_cluster_ = "";
      this->message_exp_inspect_ = this->message_exp_inspect_default_;
      this->inspect_particle_ = -1;
      this->inspect_cluster_ = -1;
      this->inspect_cluster_particle_ = -1;
      Type type = Type::TriangleCell;
      int choice = this->inject_sprite_;
      if (0 == choice) {
        type = Type::PrematureSpore;
      } else if (1 == choice) {
        type = Type::MatureSpore;
      } else if (2 == choice) {
        type = Type::Ring;
      } else if (3 == choice) {
        type = Type::PrematureCell;
      } else if (4 == choice) {
        type = Type::TriangleCell;
      } else if (5 == choice) {
        type = Type::SquareCell;
      } else if (6 == choice) {
        type = Type::PentagonCell;
      }
      this->message_exp_inject_ =
        ctrl.inject(type, this->inject_dpe_);
      log.add(Attn::O, this->message_exp_inject_);
      // TODO: sync num and dpe independently
      unsigned int num = ctrl.get_num();
      uistate.num_ = num;
      this->inject_dpe_ = static_cast<float>(num)
                         / uistate.width_ / uistate.height_;
    }
    this->auto_width(width, 3);
    ImGui::SameLine();
    ImGui::Text("sprite");
    ImGui::SameLine();
    ImGui::Combo(",", &this->inject_sprite_,
                 "premature spore\0"
                 "mature spore\0"
                 "ring\0"
                 "premature cell\0"
                 "triangle cell\0"
                 "square cell\0"
                 "pentagon cell\0\0");
    ImGui::SameLine();
    ImGui::Text("  DPE ");
    ImGui::SameLine();
    ImGui::InputFloat(".", &this->inject_dpe_, 0.01f, 10.0f, "%.5f");
    if (!this->message_exp_inject_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_inject_.c_str());
    }

    // inspection
    ImGui::BeginGroup();
    ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)0),
                      ImVec2(inspect_width, inspect_height),
                      true, ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    ImGui::Text("part.");
    ImGui::EndMenuBar();
    for (int p = 0; p < state.num_; ++p) {
      if (ImGui::Selectable(std::to_string(p).c_str(),
                            this->inspect_particle_ == p))
      {
        this->inspect_particle_ = p;
        this->inspect_cluster_= -1;
        this->inspect_cluster_particle_ = -1;
        std::vector<unsigned int> ps;
        ps.push_back(p);
        ctrl.highlight(ps);
        ctrl.color(Coloring::Inspect);
        uistate.coloring_ = Coloring::Inspect;
        uistate.deceive();
        this->gen_message_exp_inspect();
      }
    }
    ImGui::EndChild();
    if (0 < exp.clusters_.size()) {
      ImGui::SameLine();
      ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)1),
                        ImVec2(inspect_width, inspect_height),
                        true, ImGuiWindowFlags_MenuBar);
      ImGui::BeginMenuBar();
      ImGui::Text("clus.");
      ImGui::EndMenuBar();
      for (int c = 0; c < exp.clusters_.size(); ++c) {
        if (ImGui::Selectable(std::to_string(c).c_str(),
                              this->inspect_cluster_ == c))
        {
          this->inspect_particle_= -1;
          this->inspect_cluster_ = c;
          this->inspect_cluster_particle_ = -1;
          std::vector<unsigned int> ps;
          for (int p : exp.clusters_[c]) {
            ps.push_back(p);
          }
          ctrl.highlight(ps);
          ctrl.color(Coloring::Inspect);
          uistate.coloring_ = Coloring::Inspect;
          uistate.deceive();
          this->gen_message_exp_inspect();
        }
      }
      ImGui::EndChild();
    }
    if (0 <= this->inspect_cluster_) {
      inspect_width =
        std::max(inspect_width,
                 (3 + std::to_string(this->inspect_cluster_).size())
                  * font_width);
      ImGui::SameLine();
      ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)2),
                        ImVec2(inspect_width, inspect_height),
                        true, ImGuiWindowFlags_MenuBar);
      ImGui::BeginMenuBar();
      ImGui::Text("c %d", this->inspect_cluster_);
      ImGui::EndMenuBar();
      for (auto p : exp.clusters_[this->inspect_cluster_]) {
        if (ImGui::Selectable(std::to_string(p).c_str(),
                              this->inspect_cluster_particle_ == p))
        {
          this->inspect_particle_ = -1;
          // keep cluster index
          this->inspect_cluster_particle_ = p;
          std::vector<unsigned int> ps;
          ps.push_back(p);
          ctrl.highlight(ps);
          ctrl.color(Coloring::Inspect);
          uistate.coloring_ = Coloring::Inspect;
          uistate.deceive();
          this->gen_message_exp_inspect();
        }
        ++p;
      }
      ImGui::EndChild();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::TextColored(color_exp, "Inspecting:%s",
                       this->message_exp_inspect_.c_str());
    /**
    // exp: for "capturing" clusters for later injection
    if (ImGui::Button("Print cluster xyfs")) {
      for (auto p : exp.clusters_[this->inspect_cluster_]) {
        std::cout << state.px_[p] << " " << state.py_[p] << " "
                  << Util::rad_to_deg(state.pf_[p]) << std::endl;
      }
    }
    //*/

    // usage help /////////////////////////////////////////////////////////////

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Usage help");

    // pause
    ImGui::TextColored(color_dimmer, "pause/resume:");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Space");

    // step
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "step:");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "S");

    // quit
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "quit");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "C");
    this->backspace();
    ImGui::TextColored(color_dimmer, ", Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "Q");

    // save
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "save");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "S");

    // load
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "load");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "L");
    ImGui::PushFont(this->font_b);

    // capture
    ImGui::TextColored(color_dimmer, "take picture");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "P");

    // camera dolly and pivot
    ImGui::TextColored(color_dim, "               QWE");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 + 8 * this->font_width_);
    ImGui::TextColored(color_dim, "UIO");
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "camera  dolly");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "A D");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, " pivot:");
    ImGui::PopFont();
    this->backspace(-1);
    ImGui::TextColored(color_dim, " J L");
    ImGui::TextColored(color_dim, "               ZXC");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 + 8 * this->font_width_);
    ImGui::TextColored(color_dim, "M,.");
    ImGui::TextColored(color_dimmer, "            mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "L");
    ImGui::SameLine();
    ImGui::TextColored(color_dimmer, "     mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "R");

    // camera zoom
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "         zoom");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Minus");
    this->backspace();
    ImGui::TextColored(color_dimmer, "(");
    this->backspace();
    ImGui::TextColored(color_dim, "-");
    this->backspace();
    ImGui::TextColored(color_dimmer, "), ");
    this->backspace();
    ImGui::TextColored(color_dim, "Equal");
    this->backspace();
    ImGui::TextColored(color_dimmer, "(");
    this->backspace();
    ImGui::TextColored(color_dim, "=");
    this->backspace();
    ImGui::TextColored(color_dimmer, "), mouse");
    this->backspace();
    ImGui::TextColored(color_dim, "wheel");

    // camera reset
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "        reset");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Slash");
    this->backspace();
    ImGui::TextColored(color_dimmer, "(");
    this->backspace();
    ImGui::TextColored(color_dim, "/");
    this->backspace();
    ImGui::TextColored(color_dimmer, ")");

    // message box
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "messages box");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":   ");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Grave");
    this->backspace();
    ImGui::TextColored(color_dimmer, "(");
    this->backspace();
    ImGui::TextColored(color_dim, "`");
    this->backspace();
    ImGui::TextColored(color_dimmer, ")");

    // brief corner
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "brief corner");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":   ");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Apostrophe");
    this->backspace();
    ImGui::TextColored(color_dimmer, "(");
    this->backspace();
    ImGui::TextColored(color_dim, "'");
    this->backspace();
    ImGui::TextColored(color_dimmer, ")");

    // config box
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "this config box");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Escape");
  }
  ImGui::End();
}


void
Gui::draw_capture(Box box)
{
  if (Box::Capture != box) {
    return;
  }
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  int w = std::max(static_cast<int>(400.0f / this->scale_),
                   static_cast<int>(window_width * 0.75f));
  int h = std::max(static_cast<int>(250.0f / this->scale_),
                   static_cast<int>(window_height * 0.3f));
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_dim = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  static char bad_input = ' ';
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((window_width - w) / 2,
                                 (window_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin("Save PNG picture to where?", NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::Text("cwd:");
    ImGui::SameLine();
    ImGui::Text("%s", this->cwd_.c_str());
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    if (this->input_focus_) {
      ImGui::SetKeyboardFocusHere(0);
      this->input_focus_ = false;
    }
    ImGui::InputTextWithHint(this->capture_path_.c_str(), "/path/to/file",
                             path, IM_ARRAYSIZE(path));
    ImGui::PopItemWidth();
    if (this->bad_capture_) {
      ImGui::TextColored(color_bad, "  Capture failed!");
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
    if (ImGui::Button("CAPTURE",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 10, 0)))
    {
      if (std::string(path).empty()) {
        bad_input = 'e';
      } else if (!std::regex_match(path, std::regex(allowed))) {
        bad_input = 'b';
      } else {
        bad_input = ' ';
        this->capturing_ = true;
        this->capture_path_ = std::string(path);
      }
    }
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x
                                       - 20, 0))) {
      bad_input = ' ';
      //this->capture_path_.clear();
      this->bad_capture_ = false;
      this->box_ = Box::None;
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::TextColored(color_bad, "Warning:");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "existing file will be overwritten");
  }
  ImGui::End();
}


void
Gui::draw_captured(Box box)
{
  if (Box::Captured != box) {
    return;
  }
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  int w = std::max(static_cast<int>(400.0f / this->scale_),
                   static_cast<int>(window_width * 0.75f));
  int h = std::max(static_cast<int>(250.0f / this->scale_),
                   static_cast<int>(window_height * 0.3f));

  ImGui::SetNextWindowPos(ImVec2((window_width - w) / 2,
                                 (window_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin("Picture saved", NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    ImGui::Text("Successfully saved picture to: %s",
                this->capture_path_.c_str());
    ImGui::PopItemWidth();
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushFont(this->font_b);
    if (ImGui::Button("OK",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 10, 0)))
    {
      //this->capture_path_.clear();
      this->box_ = Box::None;
    }
    ImGui::PopFont();
  }
  ImGui::End();
}

void
Gui::draw_save_load(Box box)
{
  if (Box::Save != box && Box::Load != box) {
    return;
  }
  Control& ctrl = this->uistate_.ctrl_;
  Canvas& canvas = this->canvas_;
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  int w = std::max(static_cast<int>(400.0f / this->scale_),
                   static_cast<int>(window_width * 0.75f));
  int h = std::max(static_cast<int>(250.0f / this->scale_),
                   static_cast<int>(window_height * 0.3f));
  std::string title = "Save state to where?";
  std::string button = "SAVE";
  bool (UiState::*func)(const std::string&) = &UiState::save;
  if (Box::Load == box) {
    title = "Load state from where?";
    button = "LOAD";
    func = &UiState::load;
  }
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_dim = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  static char bad_input = ' ';
  static bool bad_save = false;
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((window_width - w) / 2,
                                 (window_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin(title.c_str(), NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::Text("cwd:");
    ImGui::SameLine();
    ImGui::Text("%s", this->cwd_.c_str());
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    if (this->input_focus_) {
      ImGui::SetKeyboardFocusHere(0);
      this->input_focus_ = false;
    }
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
        bad_save = !(this->uistate_.*func)(path);
        if (!bad_save) {
          this->box_ = Box::None;
          ctrl.pause(true);
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
      this->box_ = Box::None;
      ctrl.pause(true);
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::TextColored(color_bad, "Warning:");
    ImGui::SameLine();
    if (Box::Load == box) {
      ImGui::TextColored(color_dim, "system will be changed immediately");
    } else {
      ImGui::TextColored(color_dim, "existing file will be overwritten");
    }
  }
  ImGui::End();
}


void
Gui::draw_quit(Box box)
{
  if (Box::Quit != box) {
    return;
  }
  Canvas& canvas = this->canvas_;
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  int w = std::max(static_cast<int>(420.0f / this->scale_),
                   static_cast<int>(window_width * 0.75f));
  int h = std::max(static_cast<int>(460.0f / this->scale_),
                   static_cast<int>(window_width * 0.5f));
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
  auto color_dim = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  static char bad_input = ' ';
  static bool bad_save = false;
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((window_width - w) / 2,
                                 (window_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin("Save before quitting?", NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::Text("cwd:");
    ImGui::SameLine();
    ImGui::Text("%s", this->cwd_.c_str());
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
    if (this->input_focus_) {
      ImGui::SetKeyboardFocusHere(0);
      this->input_focus_ = false;
    }
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
        bad_save = !this->uistate_.save(path);
        if (!bad_save) {
          canvas.close();
        }
      }
    }
      ImGui::Text("");
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
      if (ImGui::Button("QUIT without saving",
                        ImVec2(ImGui::GetContentRegionAvail().x - 40, 0))) {
        canvas.close();
      }
      ImGui::PopFont();
      ImGui::Text("");
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
      if (ImGui::Button("Cancel",
                        ImVec2(ImGui::GetContentRegionAvail().x - 40, 0))) {
        bad_input = ' ';
        bad_save = false;
        this->box_ = Box::None;
        this->uistate_.ctrl_.pause(true);
      }
      ImGui::Text("");
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
      ImGui::TextColored(color_dim, " Ctrl+Q to QUIT without saving");
  }
  ImGui::End();
}


void
Gui::gen_message_exp_inspect()
{
  Control& ctrl = this->uistate_.ctrl_;
  State& state = ctrl.get_state();
  Exp& exp = ctrl.get_exp();
  bool no_cl = !ctrl.cl_good();
  std::vector<int>& pls = state.pls_;
  std::vector<int>& prs = state.prs_;
  std::vector<float>& pld = state.pld_;
  std::vector<float>& prd = state.prd_;
  unsigned int n_stride = state.n_stride_;
  std::ostringstream message;

  message << std::fixed << std::setprecision(3);

  if (0 <= this->inspect_cluster_particle_) {
    std::set<int>& cluster = exp.clusters_[this->inspect_cluster_];
    unsigned int cp = static_cast<unsigned int>(*cluster.find(
      this->inspect_cluster_particle_));
    message << " particle " << cp
            << " of cluster " << this->inspect_cluster_
            << "\n\ntype: " << state.type_name(state.pt_[cp])
            << "\nx: " << state.px_[cp]
            << "\ny: " << state.py_[cp]
            << "\nphi: " << Util::rad_to_deg(state.pf_[cp])
            << "\nn: " << state.pn_[cp]
            << "\nl: " << state.pl_[cp]
            << "\nr: " << state.pr_[cp];

    if (no_cl) {
      message << "\nnd: ";
      for (int i = n_stride * cp; i < n_stride * cp + n_stride; ++i) {
        if (0 > pls[i]) {
          continue;
        }
        message << pls[i] << "(" << pld[i] << ") ";
      }
      for (int i = n_stride * cp; i < n_stride * cp + n_stride; ++i) {
        if (0 > prs[i]) {
          continue;
        }
        message << prs[i] << "(" << prd[i] << ") ";
      }
    }
  }

  else if (0 <= this->inspect_cluster_) {
    unsigned int c = static_cast<unsigned int>(this->inspect_cluster_);
    std::string type = "unknown";
    if (0 < exp.cell_clusters_.count(c)) {
      type = "cell";
    } else if (0 < exp.spore_clusters_.count(c)) {
      type = "spore";
    }
    message << " cluster " << c
            << "\n\ntype: " << type
            << "\n# particles: " << exp.clusters_[c].size()
               ;
  }

  else if (0 <= this->inspect_particle_) {
    unsigned int p = static_cast<unsigned int>(this->inspect_particle_);
    message << " particle " << p
            << "\n\ntype: " << state.type_name(state.pt_[p])
            << "\nx: " << state.px_[p]
            << "\ny: " << state.py_[p]
            << "\nphi: " << Util::rad_to_deg(state.pf_[p])
            << "\nn: " << state.pn_[p]
            << "\nl: " << state.pl_[p]
            << "\nr: " << state.pr_[p];
    if (no_cl) {
      message << "\nnd: ";
      for (int i = n_stride * p; i < n_stride * p + n_stride; ++i) {
        if (0 > pls[i]) {
          continue;
        }
        message << pls[i] << "(" << pld[i] << ") ";
      }
      for (int i = n_stride * p; i < n_stride * p + n_stride; ++i) {
        if (0 > prs[i]) {
          continue;
        }
        message << prs[i] << "(" << prd[i] << ") ";
      }
    }
  }

  else {
    message << this->message_exp_inspect_default_;
  }

  this->message_exp_inspect_ = message.str();
}


void
Gui::auto_width(int width, int factor /* = 1 */)
{
  float w;
  if (1 == factor) {
    w = -0.25f * width;
  } else {
    w = ImGui::GetContentRegionAvail().x / (factor + 1);
  }
  ImGui::PushItemWidth(w);
}


void
Gui::key_callback(GLFWwindow* window, int key, int /* scancode */, int action,
                  int mods)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;
  UiState& uistate = gui->uistate_;
  Control& ctrl = uistate.ctrl_;

  if (GLFW_RELEASE == action) {
    return;
  }

  if (GLFW_PRESS == action) {
    if (mods & GLFW_MOD_CONTROL) {
      // quit
      if (Box::Quit == gui->box_) {
        if (GLFW_KEY_C == key || GLFW_KEY_Q == key) {
          canvas->close();
          return;
        }
      }
      if (GLFW_KEY_C == key || GLFW_KEY_Q == key) {
        gui->box_ = Box::Quit;
        gui->input_focus_ = true;
        ctrl.pause(true);
        return;
      }
      // load
      if (GLFW_KEY_L == key) {
        gui->box_ = Box::Load;
        gui->input_focus_ = true;
        ctrl.pause(true);
        return;
      }
      // capture
      if (GLFW_KEY_P == key) {
        gui->box_ = Box::Capture;
        gui->input_focus_ = true;
        ctrl.pause(true);
        return;
      }
      // save
      if (GLFW_KEY_S == key) {
        gui->box_ = Box::Save;
        gui->input_focus_ = true;
        ctrl.pause(true);
        return;
      }
    }
    // close box
    if (GLFW_KEY_ESCAPE == key) {
      if (Box::None == gui->box_) {
        gui->box_ = Box::Config;
        return;
      }
      gui->box_ = Box::None;
      return;
    }
    // deceive
    if (Box::None != gui->box_ && Box::Config != gui->box_) {
      return;
    }
    if (GLFW_KEY_ENTER == key) {
      ctrl.reset_exp();
      gui->message_exp_color_ = "";
      gui->message_exp_cluster_ = "";
      gui->message_exp_inject_ = "";
      gui->message_exp_inspect_ = gui->message_exp_inspect_default_;
      gui->inspect_particle_ = -1;
      gui->inspect_cluster_ = -1;
      gui->inspect_cluster_particle_ = -1;
      if (!gui->message_set_.empty()) {
        gui->log_.add(Attn::O, gui->message_set_);
        gui->message_set_ = "";
      }
      gui->inject_dpe_ = static_cast<float>(uistate.num_)
                         / uistate.width_ / uistate.height_;
      uistate.deceive();
      canvas->camera_default();
      return;
    }
    // pause
    if (GLFW_KEY_SPACE == key) { ctrl.pause(!ctrl.paused_); return; }
    // messages box
    if (GLFW_KEY_GRAVE_ACCENT == key) {
      gui->messages_ = !gui->messages_;
      return;
    }
    // brief corner
    if (GLFW_KEY_APOSTROPHE == key) { gui->brief_ = !gui->brief_; return; }
    // reset camera
    if (GLFW_KEY_SLASH == key) { canvas->camera_default(); return; }
  }

  // iterate through inspection
  Exp& exp = ctrl.get_exp();
  unsigned int num = ctrl.get_num();
  if (GLFW_KEY_DOWN == key ||
      Box::Config != gui->box_ && GLFW_KEY_RIGHT == key)
  {
    if (0 <= gui->inspect_cluster_particle_) {
      std::set<int>& cluster = exp.clusters_[gui->inspect_cluster_];
      auto i = cluster.find(gui->inspect_cluster_particle_);
      if (cluster.end() == ++i) {
        gui->inspect_cluster_particle_ = *cluster.begin();
      } else {
        gui->inspect_cluster_particle_ = *i;
      }
      std::vector<unsigned int> ps;
      ps.push_back(gui->inspect_cluster_particle_);
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
    if (0 <= gui->inspect_cluster_) {
      if (exp.clusters_.size() <= ++gui->inspect_cluster_) {
        gui->inspect_cluster_ -= exp.clusters_.size();
      }
      std::vector<unsigned int> ps;
      for (int p : exp.clusters_[gui->inspect_cluster_]) {
        ps.push_back(p);
      }
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
    if (0 <= gui->inspect_particle_) {
      if (num <= ++gui->inspect_particle_) {
        gui->inspect_particle_ -= num;
      }
      std::vector<unsigned int> ps;
      ps.push_back(gui->inspect_particle_);
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
  }
  if (GLFW_KEY_UP == key ||
      Box::Config != gui->box_ && GLFW_KEY_LEFT == key)
  {
    if (0 <= gui->inspect_cluster_particle_) {
      std::set<int>& cluster = exp.clusters_[gui->inspect_cluster_];
      std::set<int>::reverse_iterator i;
      for (i = cluster.rbegin(); i != cluster.rend(); ++i) {
        if (gui->inspect_cluster_particle_ == *i) {
          break;
        }
      }
      if (cluster.rend() == ++i) {
        gui->inspect_cluster_particle_ = *cluster.rbegin();
      } else {
        gui->inspect_cluster_particle_ = *i;
      }
      std::vector<unsigned int> ps;
      ps.push_back(gui->inspect_cluster_particle_);
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
    if (0 <= gui->inspect_cluster_) {
      if (0 > --gui->inspect_cluster_) {
        gui->inspect_cluster_ += exp.clusters_.size();
      }
      std::vector<unsigned int> ps;
      for (int p : exp.clusters_[gui->inspect_cluster_]) {
        ps.push_back(p);
      }
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
    if (0 <= gui->inspect_particle_) {
      if (0 > --gui->inspect_particle_) {
        gui->inspect_particle_ += num;
      }
      std::vector<unsigned int> ps;
      ps.push_back(gui->inspect_particle_);
      ctrl.highlight(ps);
      ctrl.color(Coloring::Inspect);
      uistate.coloring_ = Coloring::Inspect;
      uistate.deceive();
      gui->gen_message_exp_inspect();
      return;
    }
    return;
  }

  // step
  if (GLFW_KEY_S == key) { ctrl.paused_ = true; ctrl.step_ = true; return; }

  // the following for canvas only
  if (Box::None != gui->box_) {
    return;
  }

  float d = canvas->dollyd_;
  float z = canvas->zoomd_;
  float p = canvas->pivotd_;
  // camera translate x y
  if (GLFW_KEY_W == key) { canvas->camera(   0,-1*d,0,0,0); return; }
  if (GLFW_KEY_A == key) { canvas->camera(   d,   0,0,0,0); return; }
  if (GLFW_KEY_X == key) { canvas->camera(   0,   d,0,0,0); return; }
  if (GLFW_KEY_D == key) { canvas->camera(-1*d,   0,0,0,0); return; }
  if (GLFW_KEY_Q == key) { canvas->camera(   d,-1*d,0,0,0); return; }
  if (GLFW_KEY_E == key) { canvas->camera(-1*d,-1*d,0,0,0); return; }
  if (GLFW_KEY_Z == key) { canvas->camera(   d,   d,0,0,0); return; }
  if (GLFW_KEY_C == key) { canvas->camera(-1*d,   d,0,0,0); return; }
  // camera translate z
  if (GLFW_KEY_EQUAL == key) { canvas->camera(0,0,   z,0,0); return; }
  if (GLFW_KEY_MINUS == key) { canvas->camera(0,0,-1*z,0,0); return; }
  // camera rotate
  if (GLFW_KEY_I == key) { canvas->camera(0,0,0,   p,   0); return; }
  if (GLFW_KEY_J == key) { canvas->camera(0,0,0,   0,   p); return; }
  if (GLFW_KEY_COMMA == key) { canvas->camera(0,0,0,-1*p,0); return; }
  if (GLFW_KEY_L == key) { canvas->camera(0,0,0,   0,-1*p); return; }
  if (GLFW_KEY_U == key) { canvas->camera(0,0,0,   p,   p); return; }
  if (GLFW_KEY_O == key) { canvas->camera(0,0,0,   p,-1*p); return; }
  if (GLFW_KEY_M == key) { canvas->camera(0,0,0,-1*p,   p); return; }
  if (GLFW_KEY_PERIOD == key) { canvas->camera(0,0,0,-1*p,-1*p); return; }
}


void
Gui::mouse_move_callback(GLFWwindow* window, double x, double y)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;

  if (Box::None != gui->box_ || gui->messages_) {
    return;
  }

  double dx = x - gui->x_;
  double dy = y - gui->y_;
  dx *= 0.00005f;
  dy *= 0.00005f;
  if (gui->dolly_) { canvas->camera(dx,-1.0f*dy,0,0,0); return; }
  dx *= 100.0f;
  dy *= 100.0f;
  if (gui->pivot_) { canvas->camera(0,0,0,dy,dx); return; }
  gui->x_ = x;
  gui->y_ = y;
}


void
Gui::mouse_button_callback(GLFWwindow* window, int button, int action,
                           int /* mods */)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;

  if (Box::None != gui->box_ || gui->messages_) {
    return;
  }

  if (action == GLFW_PRESS) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)  { gui->dolly_ = true; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->pivot_ = true; return; }
    return;
  }
  if (action == GLFW_RELEASE) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)  { gui->dolly_ = false; return; }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { gui->pivot_ = false; return; }
    return;
  }
}


void
Gui::mouse_scroll_callback(GLFWwindow* window, double /* dx */, double dy)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;

  if (Box::None != gui->box_ || gui->messages_) {
    return;
  }

  dy *= 0.1f;
  canvas->camera(0,0,dy,0,0);
}

