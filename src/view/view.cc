#include <iostream>
#include <vector>
//#include <CL/cl.hpp>
#include <GL/glew.h>
#include "../const.hh"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "view.hh"

View::View()
{
  View::width = 800;
  View::height = 800;
  if (! (View::window = View::Init(View::width, View::height)))
  {
    std::cout << "Error: View construction" << std::endl;
  }
}

static std::string
InitEnv()
{
  // to allow version 330 shader syntax
  putenv((char*) "MESA_GL_VERSION_OVERRIDE=3.3");
  putenv((char*) "MESA_GLSL_VERSION_OVERRIDE=330");
  return "#version 330 core";
}

static GLFWwindow*
InitGlfw(unsigned int width, unsigned int height)
{
  GLFWwindow* window;
  if (! glfwInit())
  {
    std::cout << "Error: glfwInit" << std::endl;
    return NULL;
  }
  window = glfwCreateWindow(width, height, ME, NULL, NULL);
  if (! window)
  {
    std::cout << "Error: glfwCreateWindow" << std::endl;
    glfwTerminate();
    return NULL;
  }
  glfwMakeContextCurrent(window);
  return window;
}

static bool
InitGlew()
{
  if (GLEW_OK != glewInit())
  {
    std::cout << "Error: glewInit" << std::endl;
    return false;
  }
  return true;
}

static void
InitImgui(GLFWwindow* window, std::string version)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(version.c_str());
}

GLFWwindow*
View::Init(unsigned int width, unsigned int height)
{
  std::string version = InitEnv();
  GLFWwindow* window;
  if (! (window = InitGlfw(width, height)))
  {
    return NULL;
  }
  if (! InitGlew())
  {
    return NULL;
  }
  InitImgui(window, version);
  return window;
}

void
View::End()
{
  glfwTerminate();
}

static unsigned int
CompileShader(unsigned int type, const std::string& source)
{
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);
  int success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (GL_FALSE == success)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char* message = (char*) alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    std::cout << "Error: failed to compile " <<
      (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
    std::cout << message << std::endl;
    glDeleteShader(id);
    return 0;
  }
  return id;
}

static unsigned int
CreateShader(const std::string& vertexShader,
             const std::string& fragmentShader)
{
  unsigned int program = glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return program;
}

unsigned int
PrepTriangle()
{
  float vertices[6] = { -0.5f, -0.5f,
                         0.0f,  0.5f,
                         0.5f, -0.5f };
  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertices, GL_STATIC_DRAW);
  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
  std::string vertexShader = "#version 330 core\n"
    "layout(location = 0) in vec4 position;\n"
    "void main() {\n"
    "  gl_Position = position;\n"
    "}\n";
  std::string fragmentShader = "#version 330 core\n"
    "layout(location = 0) out vec4 color;\n"
    "void main() {\n"
    "  color = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";
  unsigned int program = CreateShader(vertexShader, fragmentShader);
  glUseProgram(program);
  glBindVertexArray(vao);
  return program;
}

void
DrawTriangle()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void
DeleteTriangle(unsigned int shader)
{
  glDeleteProgram(shader);
}

void
DrawGui(GLFWwindow* window)
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  {
    static int counter = 0;
    int glfw_width = 0;
    int glfw_height = 0;
    int controls_width = 0;
    glfwGetFramebufferSize(window, &glfw_width, &glfw_height);
    controls_width = glfw_width;
    if ((controls_width /= 3) < 300)
    {
      controls_width = 300;
    }
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(controls_width),
                                    static_cast<float>(glfw_height - 20)),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Application");
    ImGui::Text("Main window width: %d", glfw_width);
    ImGui::Text("Main window height: %d", glfw_height);
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
ProcessInputKey(GLFWwindow *window)
{
  if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
  {
    glfwSetWindowShouldClose(window, true);
  }
  if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ENTER))
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }
}

int
View::Draw(GLFWwindow* window)
{
  unsigned int program = PrepTriangle();
  while (! glfwWindowShouldClose(window))
  {
    //ProcessInputKey(window);
    DrawTriangle();
    DrawGui(window);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  DeleteTriangle(program);
  return 0;
}

