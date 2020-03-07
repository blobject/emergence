#include <iostream>
#include <GLFW/glfw3.h>

#include "gui.hh"
#include "../util/common.hh"


Gui::Gui(const std::string &version, unsigned int width, unsigned int height)
{
  // glfw
  GLFWwindow* window;
  if (! glfwInit())
  {
    std::cerr << "Error: glfwInit" << std::endl;
    return;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(width, height, ME, NULL, NULL);
  if (! window)
  {
    std::cerr << "Error: glfwCreateWindow" << std::endl;
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
  glfwTerminate();
}


void
Gui::Close()
{
  glfwSetWindowShouldClose(this->window_, true);
}


bool
Gui::Closing()
{
  return glfwWindowShouldClose(this->window_);
}


void
Gui::Draw()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  {
    static int counter = 0;
    int window_width = 0;
    int window_height = 0;
    int controls_width = 0;
    glfwGetFramebufferSize(this->window_, &window_width, &window_height);
    controls_width = window_width;
    if ((controls_width /= 3) < 300)
    {
      controls_width = 300;
    }
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(controls_width),
                                    static_cast<float>(window_height - 20)),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Application");
    ImGui::Text("Main window width: %d", window_width);
    ImGui::Text("Main window height: %d", window_height);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "GLFW");
    ImGui::Text("%s", glfwGetVersionString());
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    if (ImGui::Button("Counter button"))
    {
      std::cout << "counter button clicked\n";
      counter++;
      if (counter == 9) { ImGui::OpenPopup("Easter egg"); }
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    if (ImGui::BeginPopupModal("Easter egg", NULL))
    {
      ImGui::Text("Ho-ho, you found me!");
      if (ImGui::Button("Buy Ultimate Orb")) { ImGui::CloseCurrentPopup(); }
      ImGui::EndPopup();
    }
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

