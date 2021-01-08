#include "gui.hh"
#include "../util/common.hh"

#include <regex>


GuiState::GuiState(Control& ctrl)
  : ctrl_(ctrl)
{
    State& truth = ctrl.get_state();
    this->stop_ = ctrl.stop_;
    this->num_ = truth.num_;
    this->width_ = truth.width_;
    this->height_ = truth.height_;
    this->alpha_ = Util::rad_to_deg(truth.alpha_);
    this->beta_ = Util::rad_to_deg(truth.beta_);
    this->scope_ = truth.scope_;
    this->speed_ = truth.speed_;
    this->colors_ = truth.colors_;
    this->preset_ = 0;
}


bool
GuiState::untrue() const
{
    // TODO: stop
    Stative gui_state = {this->stop_,
                         this->num_,
                         this->width_,
                         this->height_,
                         Util::deg_to_rad(this->alpha_),
                         Util::deg_to_rad(this->beta_),
                         this->scope_,
                         this->speed_,
                         this->colors_};
    return this->ctrl_.different(gui_state);
}


bool
GuiState::deceive() const
{
  Stative gui_state = {this->stop_,
                       this->num_,
                       this->width_,
                       this->height_,
                       Util::deg_to_rad(this->alpha_),
                       Util::deg_to_rad(this->beta_),
                       this->scope_,
                       this->speed_,
                       this->colors_};
  // truth change provokes Canvas (observer) reaction
  return this->ctrl_.change(gui_state);
}


void
GuiState::random(Log& log)
{
    this->alpha_ = Util::distribute<float>(-180.0f, 180.0f);
    this->beta_ = Util::distribute<float>(-180.0f, 180.0f);
    this->scope_ = Util::distribute<float>(1.0f, 96.0f);
    this->speed_ = Util::distribute<float>(1.0f, 32.0f);
    log.add(Attn::O, "Random: a=" + std::to_string(this->alpha_)
            + ", b=" + std::to_string(this->beta_)
            + ", v=" + std::to_string(this->scope_)
            + ", s=" + std::to_string(this->speed_));
    this->deceive();
}


void
GuiState::preset(Log& log)
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
    log.add(Attn::O, preset
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
    this->stop_   = loaded.stop;
    this->num_    = loaded.num;
    this->width_  = loaded.width;
    this->height_ = loaded.height;
    this->alpha_  = Util::rad_to_deg(loaded.alpha);
    this->beta_   = Util::rad_to_deg(loaded.beta);
    this->scope_  = loaded.scope;
    this->speed_  = loaded.speed;
    this->colors_ = loaded.colors;
    return true;
}


Gui::Gui(Log& log, GuiState state, Canvas& canvas,
         unsigned int width, unsigned int height, bool hide_side)
    : canvas_(canvas), state_(state), log_(log), side_(!hide_side)
{
    // glfw
    GLFWwindow* view;
    if (!glfwInit()) {
        log.add(Attn::Egl, "glfwInit");
        return;
    }
    unsigned int gui_width = width;
    unsigned int gui_height = height;
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
    glfwSetMouseButtonCallback(view, mouse_button_callback);
    glfwSetCursorPosCallback(view, mouse_move_callback);
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
    float font_size = 20.0f;

    this->view_ = view;
    std::string cwd = Util::emergence_dir();
    this->font_r = io.Fonts->AddFontFromFileTTF(
        (cwd + "/../opt/roboto/RobotoMono-Regular.ttf").c_str(), font_size);
    this->font_b = io.Fonts->AddFontFromFileTTF(
        (cwd + "/../opt/roboto/RobotoMono-Bold.ttf").c_str(), font_size);
    this->font_i = io.Fonts->AddFontFromFileTTF(
        (cwd + "/../opt/roboto/RobotoMono-Italic.ttf").c_str(), font_size);
    this->font_z = io.Fonts->AddFontFromFileTTF(
        (cwd + "/../opt/roboto/RobotoMono-BoldItalic.ttf").c_str(), font_size);
    this->gui_width_ = gui_width;
    this->gui_height_ = gui_height;
    this->console_ = false;
    this->dialog_ = ' ';
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

    this->draw_side(this->side_);
    this->draw_console(this->console_);
    this->draw_save_load(this->dialog_);
    this->draw_quit(this->dialog_);
    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void
Gui::draw_side(bool draw)
{
    if (!draw) {
        return;
    }
    Log& log = this->log_;
    GuiState& state = this->state_;
    Canvas& canvas = this->canvas_;
    bool untrue = state.untrue();
    float width = 24.0f * this->font_width_;
    int view_width;
    int view_height;
    glfwGetFramebufferSize(this->view_, &view_width, &view_height);
    int right_alignment = width - 8;
    int button_width = 5 * this->font_width_;
    int random_width = 7 * this->font_width_;
    auto button_size = ImVec2(button_width, 0);
    auto random_size = ImVec2(random_width, 0);
    auto color_status = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
    auto color_title = ImVec4(1.0f, 0.75f, 0.25f, 1.0f);
    auto color_dim = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
    auto color_dimmer = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    const ImS64 negone = -1;
    const ImU32 zero = 0;
    const ImU32 max_dim = 100000;
    const ImU32 max_num = 1000000;
    const ImS64 max_stop = 2000000000;
    std::string status;
    std::string pause = "Pause";
    bool hard_paused = canvas.hard_paused_;
    static bool three = true;
    if (hard_paused || canvas.paused_) {
        pause = "Resume";
    }

    ImGui::SetNextWindowPos(ImVec2(view_width - width, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, static_cast<float>(view_height)),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);
    if (ImGui::Begin("control", NULL,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        if (hard_paused) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.5f, 0.25f, 0.25f, 1.0f));
        }
        if (ImGui::Button(pause.c_str())) {
            this->pause();
        }
        if (hard_paused) {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            this->dialog_ = 's';
            this->canvas_.hard_pause(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            this->dialog_ = 'l';
            this->canvas_.hard_pause(true);
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(right_alignment - button_width);
        if (ImGui::Button("Quit", button_size)) {
            this->dialog_ = 'q';
            this->canvas_.hard_pause(true);
        }
        if (untrue) {
            ImGui::PushFont(this->font_z);
            status = "Parameter modified";
            ImGui::SetCursorPosX(right_alignment
                                 - ImGui::CalcTextSize(status.c_str()).x);
            ImGui::TextColored(color_status, "%s", status.c_str());
            ImGui::PopFont();
        } else {
            ImGui::PushFont(this->font_i);
            status = "Parameters in sync";
            ImGui::SetCursorPosX(right_alignment
                                 - ImGui::CalcTextSize(status.c_str()).x);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.f), "%s",
                               status.c_str());
            ImGui::PopFont();
        }
        ImGui::PushFont(this->font_b);
        ImGui::TextColored(color_title, "Particle");
        if (untrue) {
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
        ImGui::InputScalar("n", ImGuiDataType_U32, &state.num_, &zero,
                           &max_num);
        ImGui::Dummy(ImVec2(0.0f, 1.0f));
        ImGui::PushFont(this->font_b);
        ImGui::TextColored(color_title, "World");
        ImGui::PopFont();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("width "); ImGui::SameLine();
        ImGui::InputScalar("w", ImGuiDataType_U32, &state.width_, &zero,
                           &max_dim);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("height"); ImGui::SameLine();
        ImGui::InputScalar("h", ImGuiDataType_U32, &state.height_, &zero,
                           &max_dim);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - random_width
                             - 24);
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
        ImGui::SameLine();
        if (ImGui::Button("Random", random_size)) {
            state.random(log);
        }
        ImGui::AlignTextToFramePadding();
        ImGui::Text("alpha "); ImGui::SameLine();
        ImGui::InputFloat("a", &state.alpha_, -180.0f, 180.0f, "%.3f");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("beta  "); ImGui::SameLine();
        ImGui::InputFloat("b", &state.beta_, -180.0f, 180.0f, "%.3f");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("scope "); ImGui::SameLine();
        ImGui::InputFloat("v", &state.scope_, 1.0f, 256.0f, "%.3f");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("speed "); ImGui::SameLine();
        ImGui::InputFloat("s", &state.speed_, 1.0f, 64.0f, "%.3f");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("stop  "); ImGui::SameLine();
        ImGui::InputScalar("!", ImGuiDataType_S64, &canvas.ctrl_.stop_, &negone,
                           &max_stop);
        ImGui::Text("colors: %d", state.colors_);
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
        if (ImGui::Checkbox("3d", &three)) {
            canvas.three(three);
        }
        ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
        ImGui::TextColored(color_dimmer, "opencl: ");
        ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
        ImGui::TextColored(color_dim, "%s",
                           canvas.ctrl_.cl_good() ? "on" : "off");
        ImGui::Dummy(ImVec2(0.0f, 1.0f));
        ImGui::PushFont(this->font_b);
        ImGui::TextColored(color_title, "Usage");
        ImGui::PopFont();
        ImGui::TextColored(color_dimmer, "quit:    ");
        ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
        ImGui::TextColored(color_dim, "Ctrl+Q");
        ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
        ImGui::TextColored(color_dimmer, "/Ctrl+");
        ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8);
        ImGui::TextColored(color_dim, "C");
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
Gui::draw_console(bool draw)
{
    if (!draw) {
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
    std::deque<std::pair<Attn,std::string>>& messages = this->log_.messages_;
    unsigned int count = messages.size();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);
    if (ImGui::Begin("console", NULL,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        for (std::pair<Attn,std::string>& message : messages) {
            if      (message.first == Attn::E)   { color = color_e; }
            else if (message.first == Attn::Ecl) { color = color_ecl; }
            else if (message.first == Attn::Egl) { color = color_egl; }
            if (message.first == Attn::O) {
                ImGui::Text("%d: %s", count, message.second.c_str());
            } else {
                ImGui::TextColored(color, "%d: %s", count,
                                   message.second.c_str());
            }
            --count;
        }
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
    int w = 400;
    int h = 220;
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

    ImGui::SetNextWindowPos(ImVec2((view_width - w) / 2,
                                   (view_height - h) / 3));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    if (ImGui::Begin(title.c_str(), NULL,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("");
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20);
        ImGui::InputTextWithHint("", "/path/to/file", path, IM_ARRAYSIZE(path));
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
                    this->canvas_.hard_pause(false);
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
            this->canvas_.hard_pause(false);
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
    int w = 400;
    int h = 380;
    static char path[128];
    auto color_bad = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
    static char bad_input = ' ';
    static bool bad_save = false;
    std::string allowed = "[/ A-Za-z0-9!@#%()\\[\\],.=+_-]+";

    ImGui::SetNextWindowPos(ImVec2((view_width - w) / 2,
                                   (view_height - h) / 3));
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
            this->canvas_.hard_pause(false);
        }
        ImGui::Text("");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                           " Ctrl+Q to QUIT without saving");
    }
    ImGui::End();
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
    if (this->canvas_.hard_paused_) {
        return;
    }
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
            if (key == GLFW_KEY_C || key == GLFW_KEY_Q) {
                if ('q' == gui->dialog_) {
                    gui->close();
                    return;
                }
                gui->dialog_ = 'q';
                canvas.hard_pause(true);
                return;
            }
            if (key == GLFW_KEY_L) {
                gui->dialog_ = 'l';
                canvas.hard_pause(true);
                return;
            }
            if (key == GLFW_KEY_S) {
                gui->dialog_ = 's';
                canvas.hard_pause(true);
                return;
            }
        }
        if (key == GLFW_KEY_ESCAPE) {
            gui->dialog_ = ' ';
            canvas.hard_pause(false);
            return;
        }
        if (key == GLFW_KEY_ENTER) { gui->state_.deceive(); return; }
        if (key == GLFW_KEY_SPACE) { gui->pause(); return; }
        if (key == GLFW_KEY_TAB) { gui->side_ = !gui->side_; return; }
        if (key == GLFW_KEY_GRAVE_ACCENT) {
            gui->console_ = !gui->console_; return; }
        if (key == GLFW_KEY_SLASH) { canvas.camera_default(); return; }
    }

    if (canvas.hard_paused_) {
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
Gui::mouse_button_callback(GLFWwindow* view, int button, int action,
                           int /* mods */)
{
    Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));

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
Gui::mouse_move_callback(GLFWwindow* view, double x, double y)
{
    Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
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
Gui::mouse_scroll_callback(GLFWwindow* view, double /* dx */, double dy)
{
    Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
    dy *= 0.1f;
    gui->canvas_.camera(0,0,dy,0,0);
}


void
Gui::resize_callback(GLFWwindow* view, int w, int h)
{
    Gui* gui = static_cast<Gui*>(glfwGetWindowUserPointer(view));
    gui->canvas_.camera_resize(w, h);
}

