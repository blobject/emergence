#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "gui.hh"
#include "../util/util.hh"


Gui::Gui(const std::string &version, unsigned int width, unsigned int height)
{
  // glfw
  GLFWwindow* window;
  if (! glfwInit())
  {
    Util::Err("glfwInit");
    return;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(width, height, ME, NULL, NULL);
  if (! window)
  {
    Util::Err("glfwCreateWindow");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(version.c_str());

  this->window_ = window;
}


Gui::~Gui()
{
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}


void
Gui::Close()
{
  glfwSetWindowShouldClose(this->window_, true);
}


bool
Gui::Closing() const
{
  return glfwWindowShouldClose(this->window_);
}


void
Gui::Draw(const State &state) const
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  {
    static int counter = 0;
    int frame_width = 200;
    int window_width = 0;
    int window_height = 0;
    int controls_width = 0;
    glfwGetFramebufferSize(this->window_, &window_width, &window_height);
    //if ((frame_width /= 4) < 200)
    //{
    //  frame_width = 200;
    //}
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(frame_width),
                                    static_cast<float>(window_height - 20)),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("Configuration", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "World");
    ImGui::Text("width:        %u", window_width);
    ImGui::Text("height:       %u", window_height);
    ImGui::Text("alpha:        %.2f", state.alpha_);
    ImGui::Text("beta:         %.2f", state.beta_);
    ImGui::Text("gamma:        %.2f", state.gamma_);
    ImGui::Text("distribution: %d", (int) state.distribution_);
    ImGui::Text("stop:         %u", state.stop_);
    ImGui::Text("color scheme: %d", state.colorscheme_);
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Particles");
    ImGui::Text("# particles:  %u", static_cast<unsigned>(state.particles_.size()));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "History");
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::End();
  }
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void
Gui::HandleInput()
{
  if (GLFW_PRESS == glfwGetKey(this->window_, GLFW_KEY_ESCAPE) ||
      GLFW_PRESS == glfwGetKey(this->window_, GLFW_KEY_Q))
  {
    this->Close();
  }
}


void
Gui::Next() const
{
  glfwSwapBuffers(this->window_);
  glfwPollEvents();
}

