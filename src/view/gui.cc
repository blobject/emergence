#include "gui.hh"
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
  std::string font = Util::emergence_dir() + "/../opt/fonts/LiberationMono-";
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
  this->dialog_ = Dialog::None;
  this->ago_ = glfwGetTime();
  this->frames_ = 0;
  this->fps_ = 0.0f;
  this->x_ = 0.0;
  this->y_ = 0.0;
  this->dolly_ = false;
  this->pivot_ = false;
  this->capturing_ = false;
  this->bad_capture_ = false;
  this->cluster_radius_ = uistate.scope_;
  this->cluster_minpts_ = 14; // avg size of premature spore (Schmickl et al.)
  this->inject_sprite_ = 4; // triangle cell
  this->inject_dpe_ = static_cast<float>(uistate.num_)
                      / uistate.width_ / uistate.height_;
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

  if (this->capturing_) {
    // TODO: not capturing correctly (tearing, dialog showing)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    this->capturing_ = false;
    int w;
    int h;
    glfwGetFramebufferSize(this->window_, &w, &h);
    if (!Image::out(this->capture_path_, w, h)) {
      this->bad_capture_ = true;
      this->dialog_ = Dialog::Capture;
      return;
    }
    this->dialog_ = Dialog::Captured;
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  this->font_width_ = ImGui::CalcTextSize(" ").x;
  this->draw_brief(this->brief_);
  this->draw_messages(this->messages_);
  this->draw_config(this->dialog_);
  this->draw_capture(this->dialog_);
  this->draw_captured(this->dialog_);
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
  int width;
  int height;
  glfwGetFramebufferSize(this->window_, &width, &height);
  auto color = ImVec4(0.75f, 0.75f, 0.75f, 0.75f);
  auto color_b = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const char* status = "running";
  if (this->canvas_.paused_) {
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
    ImGui::TextColored(color_b, "%s", status);
    ImGui::SameLine();
    ImGui::PopFont();
    ImGui::TextColored(color, "(");
    this->backspace(2);
    ImGui::PushFont(this->font_i);
    ImGui::TextColored(color, "Space");
    ImGui::SameLine();
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color, ")");
    ImGui::TextColored(color, "x");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", this->x_);
    ImGui::SameLine();
    ImGui::PopFont();
    ImGui::TextColored(color, "y");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.0f", height - this->y_);
    ImGui::PopFont();
    ImGui::TextColored(color, "fps");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%.1f", this->fps_);
    ImGui::PopFont();
    ImGui::TextColored(color, "opencl");
    ImGui::SameLine();
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_b, "%s",
                       this->uistate_.ctrl_.cl_good() ? "on" : "off");
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
  ImGui::SetNextWindowBgAlpha(0.85f);
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
Gui::draw_config(Dialog dialog)
{
  if (Dialog::Config != dialog) {
    return;
  }
  Log& log = this->log_;
  UiState& uistate = this->uistate_;
  Canvas& canvas = this->canvas_;
  int untrue = uistate.untrue();
  float margin = 4.0f * this->font_width_;
  int window_width;
  int window_height;
  glfwGetFramebufferSize(this->window_, &window_width, &window_height);
  float width = window_width - 2.0f * margin;
  float height = window_height - 2.0f * margin;
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
  if (canvas.paused_) {
    pause = "Resume";
  }
  std::string status;

  ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.85f);
  if (ImGui::Begin("config", NULL,
                   ImGuiWindowFlags_HorizontalScrollbar |
                   ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoResize)) {
    if (ImGui::Button(pause.c_str())) {
      canvas.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      this->dialog_ = Dialog::Save;
      if (!canvas_.paused_) {
        canvas.pause();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
      this->dialog_ = Dialog::Load;
      if (!canvas.paused_) {
        canvas.pause();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Take picture")) {
      this->dialog_ = Dialog::Capture;
      if (!canvas.paused_) {
        canvas.pause();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit")) {
      this->dialog_ = Dialog::Quit;
      if (!canvas.paused_) {
        canvas.pause();
      }
    }
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
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Graphics");
    ImGui::PopFont();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("space ");
    ImGui::SameLine();
    if (ImGui::Checkbox("3D", &this->three_)) {
      canvas.three(this->three_);
    }
    ImGui::AlignTextToFramePadding();
    ImGui::Text("p. rad"); ImGui::SameLine();
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("r", &uistate.prad_, 0.5f, 64.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Habitat");
    ImGui::PopFont();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("stop  ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("!", ImGuiDataType_S64, &uistate.stop_,
                       &negone, &max_stop);
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("num   ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("n", ImGuiDataType_U32, &uistate.num_, &zero, &max_num);
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("width ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("w", ImGuiDataType_U32, &uistate.width_, &zero,
                       &max_dim);
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("height");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputScalar("h", ImGuiDataType_U32, &uistate.height_, &zero,
                       &max_dim);
    ImGui::PopItemWidth();
    if (ImGui::Button("Random pattern")) {
      uistate.random(log);
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
      uistate.pattern(log);
    }
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("alpha ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("a", &uistate.alpha_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("beta  ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("b", &uistate.beta_, -180.0f, 180.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("scope ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("v", &uistate.scope_, 1.0f, 256.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("speed ");
    ImGui::SameLine();
    this->auto_width(width);
    ImGui::InputFloat("s", &uistate.speed_, 1.0f, 64.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Analysis");
    ImGui::PopFont();
    if (ImGui::Button("Reset experiment module & colors")) {
      uistate.ctrl_.reset_exp();
      this->message_exp_cluster_ = "";
      this->message_exp_inject_ = "";
      this->message_exp_density_ = "";
      uistate.coloring(log, Coloring::Normal);
    }
    if (ImGui::Button("Count clusters")) {
      this->message_exp_inject_ = "";
      this->message_exp_density_ = "";
      this->message_exp_cluster_ =
        uistate.ctrl_.cluster(this->cluster_radius_, this->cluster_minpts_);
      log.add(Attn::O, this->message_exp_cluster_);
      uistate.coloring(log, Coloring::Cluster);
    }
    this->auto_width(width, 3);
    ImGui::SameLine();
    ImGui::Text(" radius");
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
    if (ImGui::Button("Inject clusters")) {
      this->message_exp_cluster_ = "";
      this->message_exp_density_ = "";
      Sprite sprite = Sprite::TriangleCell;
      int choice = this->inject_sprite_;
      if (0 == choice) {
        sprite = Sprite::PrematureSpore;
      } else if (1 == choice) {
        sprite = Sprite::MatureSpore;
      } else if (2 == choice) {
        sprite = Sprite::Ring;
      } else if (3 == choice) {
        sprite = Sprite::PrematureCell;
      } else if (4 == choice) {
        sprite = Sprite::TriangleCell;
      } else if (5 == choice) {
        sprite = Sprite::SquareCell;
      } else if (6 == choice) {
        sprite = Sprite::PentagonCell;
      }
      this->message_exp_inject_ =
        uistate.ctrl_.inject(sprite, this->inject_dpe_);
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
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Density map ");
    ImGui::SameLine();
    this->auto_width(width);
    if (ImGui::Combo("d", &this->density_threshold_,
                     "none\0"
                     "n = 10\0"
                     "n = 15\0"
                     "n = 20\0"
                     "n = 25\0"
                     "n = 30\0\0")) {
      this->message_exp_cluster_ = "";
      this->message_exp_inject_ = "";
      Coloring scheme = Coloring::Normal;
      int choice = this->density_threshold_;
      if (0 == choice) {
        scheme = Coloring::Normal;
        this->message_exp_density_ = "";
      } else if (1 == choice) {
        scheme = Coloring::Density10;
        this->message_exp_density_ = "density threshold set to 10";
      } else if (2 == choice) {
        scheme = Coloring::Density15;
        this->message_exp_density_ = "density threshold set to 15";
      } else if (3 == choice) {
        scheme = Coloring::Density20;
        this->message_exp_density_ = "density threshold set to 20";
      } else if (4 == choice) {
        scheme = Coloring::Density25;
        this->message_exp_density_ = "density threshold set to 25";
      } else if (5 == choice) {
        scheme = Coloring::Density30;
        this->message_exp_density_ = "density threshold set to 30";
      }
      uistate.coloring(log, scheme);
    }
    ImGui::PopItemWidth();
    if (!this->message_exp_density_.empty()) {
      ImGui::TextColored(color_exp, "%s", this->message_exp_density_.c_str());
    }
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_title, "Usage help");
    ImGui::TextColored(color_dimmer, "pause/resume:");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "Space");
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
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "save");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "S");
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "load");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "L");
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "take picture");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ": Ctrl+");
    this->backspace();
    ImGui::TextColored(color_dim, "P");
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
    ImGui::PushFont(this->font_b);
    ImGui::TextColored(color_dimmer, "brief corner");
    ImGui::PopFont();
    this->backspace();
    ImGui::TextColored(color_dimmer, ":   ");
    ImGui::SameLine();
    ImGui::TextColored(color_dim, "S");
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
Gui::draw_capture(Dialog dialog)
{
  if (Dialog::Capture != dialog) {
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
  static char bad_input = ' ';
  std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

  ImGui::SetNextWindowPos(ImVec2((window_width - w) / 2,
                                 (window_height - h) / 3));
  ImGui::SetNextWindowSize(ImVec2(w, h));
  if (ImGui::Begin("Save PNG picture to where?", NULL,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
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
      this->dialog_ = Dialog::None;
    }
  }
  ImGui::End();
}


void
Gui::draw_captured(Dialog dialog)
{
  if (Dialog::Captured != dialog) {
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
      this->dialog_ = Dialog::None;
    }
    ImGui::PopFont();
  }
  ImGui::End();
}

void
Gui::draw_save_load(Dialog dialog)
{
  if (Dialog::Save != dialog && Dialog::Load != dialog) {
    return;
  }
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
  if (Dialog::Load == dialog) {
    title = "Load state from where?";
    button = "LOAD";
    func = &UiState::load;
  }
  static char path[128];
  auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
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
        bad_save = !(this->uistate_.*func)(path);
        if (!bad_save) {
          this->dialog_ = Dialog::None;
          if (!canvas.paused_) {
            canvas.pause();
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
      this->dialog_ = Dialog::None;
      canvas.pause();
    }
  }
  ImGui::End();
}


void
Gui::draw_quit(Dialog dialog)
{
  if (Dialog::Quit != dialog) {
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
        this->dialog_ = Dialog::None;
        canvas.pause();
      }
      ImGui::Text("");
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                         " Ctrl+Q to QUIT without saving");
  }
  ImGui::End();
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

  if (action == GLFW_PRESS) {
    if (mods & GLFW_MOD_CONTROL) {
      if (Dialog::Quit == gui->dialog_) {
        if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
          canvas->close();
          return;
        }
      }
      if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
        gui->dialog_ = Dialog::Quit;
        if (!canvas->paused_) {
          canvas->pause();
        }
        return;
      }
      if (key == GLFW_KEY_L) {
        gui->dialog_ = Dialog::Load;
        if (!canvas->paused_) {
          canvas->pause();
        }
        return;
      }
      if (key == GLFW_KEY_S) {
        gui->dialog_ = Dialog::Save;
        if (!canvas->paused_) {
          canvas->pause();
        }
        return;
      }
    }
    if (key == GLFW_KEY_ESCAPE) {
      if (Dialog::None == gui->dialog_) {
        gui->dialog_ = Dialog::Config;
        return;
      }
      gui->dialog_ = Dialog::None;
      return;
    }
    if (Dialog::None != gui->dialog_ && Dialog::Config != gui->dialog_) {
      return;
    }
    if (key == GLFW_KEY_ENTER) {
      gui->uistate_.deceive();
      gui->inject_dpe_ = static_cast<float>(gui->uistate_.num_)
                         / gui->uistate_.width_ / gui->uistate_.height_;
      return;
    }
    if (key == GLFW_KEY_SPACE) { canvas->pause(); return; }
    if (key == GLFW_KEY_GRAVE_ACCENT) {
      gui->messages_ = !gui->messages_;
      return;
    }
    if (key == GLFW_KEY_SLASH) { canvas->camera_default(); return; }
    if (key == GLFW_KEY_S) { gui->brief_ = !gui->brief_; return; }
  }

  if (Dialog::None != gui->dialog_) {
    return;
  }

  if (action == GLFW_RELEASE) {
    return;
  }

  float d = canvas->dollyd_;
  float z = canvas->zoomd_;
  float p = canvas->pivotd_;
  // camera translate x y
  if (key == GLFW_KEY_W) { canvas->camera(   0,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_A) { canvas->camera(   d,   0,0,0,0); return; }
  if (key == GLFW_KEY_X) { canvas->camera(   0,   d,0,0,0); return; }
  if (key == GLFW_KEY_D) { canvas->camera(-1*d,   0,0,0,0); return; }
  if (key == GLFW_KEY_Q) { canvas->camera(   d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_E) { canvas->camera(-1*d,-1*d,0,0,0); return; }
  if (key == GLFW_KEY_Z) { canvas->camera(   d,   d,0,0,0); return; }
  if (key == GLFW_KEY_C) { canvas->camera(-1*d,   d,0,0,0); return; }
  // camera translate z
  if (key == GLFW_KEY_EQUAL) { canvas->camera(0,0,   z,0,0); return; }
  if (key == GLFW_KEY_MINUS) { canvas->camera(0,0,-1*z,0,0); return; }
  // camera rotate
  if (key == GLFW_KEY_I) { canvas->camera(0,0,0,   p,   0); return; }
  if (key == GLFW_KEY_J) { canvas->camera(0,0,0,   0,   p); return; }
  if (key == GLFW_KEY_COMMA) { canvas->camera(0,0,0,-1*p,0); return; }
  if (key == GLFW_KEY_L) { canvas->camera(0,0,0,   0,-1*p); return; }
  if (key == GLFW_KEY_U) { canvas->camera(0,0,0,   p,   p); return; }
  if (key == GLFW_KEY_O) { canvas->camera(0,0,0,   p,-1*p); return; }
  if (key == GLFW_KEY_M) { canvas->camera(0,0,0,-1*p,   p); return; }
  if (key == GLFW_KEY_PERIOD) { canvas->camera(0,0,0,-1*p,-1*p); return; }
}


void
Gui::mouse_move_callback(GLFWwindow* window, double x, double y)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;

  if (Dialog::None != gui->dialog_ || gui->messages_) {
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

  if (Dialog::None != gui->dialog_ || gui->messages_) {
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
Gui::mouse_scroll_callback(GLFWwindow* window, double /* dx */, double dy)
{
  Canvas* canvas = static_cast<Canvas*>(glfwGetWindowUserPointer(window));
  Gui* gui = canvas->gui_;

  if (Dialog::None != gui->dialog_ || gui->messages_) {
    return;
  }

  dy *= 0.1f;
  canvas->camera(0,0,dy,0,0);
}

