#include <iostream>
//#include <CL/cl.hpp>
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "visualiser.hh"
#include "../util/common.hh"
#include "../util/util.hh"

Visualiser::Visualiser(Processor* proc, bool hide_ctrl)
{
  this->width_ = 800;
  this->height_ = 800;
  if (! (this->window_ = this->Init(this->width_,
                                    this->height_)))
  {
    std::cerr << "Error: Visualiser construction" << std::endl;
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
    std::cerr << "Error: glfwInit" << std::endl;
    return NULL;
  }
  window = glfwCreateWindow(width, height, ME, NULL, NULL);
  if (! window)
  {
    std::cerr << "Error: glfwCreateWindow" << std::endl;
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
    std::cerr << "Error: glewInit" << std::endl;
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
Visualiser::Init(unsigned int width, unsigned int height)
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
Visualiser::Fin()
{
  glfwTerminate();
}

static unsigned int
CompileShader(unsigned int type, const std::string& source)
{
  DOGL(unsigned int id = glCreateShader(type));
  const char* src = source.c_str();
  DOGL(glShaderSource(id, 1, &src, nullptr));
  DOGL(glCompileShader(id));
  int success;
  DOGL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));
  if (GL_FALSE == success)
  {
    int length;
    DOGL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    char* message = (char*) alloca(length * sizeof(char));
    DOGL(glGetShaderInfoLog(id, length, &length, message));
    std::cerr << "Error: failed to compile " <<
      (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << '\n';
    std::cerr << message << std::endl;
    DOGL(glDeleteShader(id));
    return 0;
  }
  return id;
}

static unsigned int
CreateShader(const std::string& vertex_shader,
             const std::string& fragment_shader)
{
  DOGL(unsigned int program = glCreateProgram());
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);
  DOGL(glAttachShader(program, vs));
  DOGL(glAttachShader(program, fs));
  DOGL(glLinkProgram(program));
  DOGL(glValidateProgram(program));
  DOGL(glDeleteShader(vs));
  DOGL(glDeleteShader(fs));
  return program;
}

static unsigned int
PrepTri()
{
  float vertices[] = { -0.5f, -0.5f,
                        0.0f,  0.5f,
                        0.5f, -0.5f};
  unsigned int vbo;
  DOGL(glGenBuffers(1, &vbo));
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  DOGL(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), vertices,
                     GL_STATIC_DRAW));
  unsigned int vao;
  DOGL(glGenVertexArrays(1, &vao));
  DOGL(glBindVertexArray(vao));
  DOGL(glEnableVertexAttribArray(0));
  DOGL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
  ShaderSource source = Util::LoadShader("../etc/basic.shader");
  unsigned int program = CreateShader(source.Vertex, source.Fragment);
  DOGL(glUseProgram(program));
  DOGL(glBindVertexArray(vao));
  return program;
}

static void
DrawTri()
{
  DOGL(glDrawArrays(GL_TRIANGLES, 0, 3));
}

static unsigned int
PrepQuad()
{
  float vertices[] = { -0.5f, -0.5f,
                        0.5f, -0.5f,
                        0.5f,  0.5f,
                       -0.5f,  0.5f};
  unsigned short indices[] = { 0, 1, 2,
                               2, 3, 0 };
  unsigned int vbo;
  DOGL(glGenBuffers(1, &vbo));
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  DOGL(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), vertices,
                     GL_STATIC_DRAW));
  unsigned int vao;
  DOGL(glGenVertexArrays(1, &vao));
  DOGL(glBindVertexArray(vao));
  DOGL(glEnableVertexAttribArray(0));
  DOGL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
  unsigned int ibo;
  DOGL(glGenBuffers(1, &ibo));
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
  DOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short),
                     indices, GL_STATIC_DRAW));
  ShaderSource source = Util::LoadShader("../etc/basic.shader");
  unsigned int program = CreateShader(source.Vertex, source.Fragment);
  DOGL(glUseProgram(program));
  DOGL(glBindVertexArray(vao));
  return program;
}

static void
DrawQuad()
{
  DOGL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));
}

static void
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
ProcessInputKey(GLFWwindow* window)
{
  if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
  {
    glfwSetWindowShouldClose(window, true);
  }
  //if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ENTER))
  //{
  //  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  //  glClear(GL_COLOR_BUFFER_BIT);
  //}
}

int
Visualiser::Draw(GLFWwindow* window)
{
  unsigned int program;
  //program = PrepTri();
  program = PrepQuad();
  while (! glfwWindowShouldClose(window))
  {
    ProcessInputKey(window);
    DOGL(glClear(GL_COLOR_BUFFER_BIT));

    //DrawTri();
    DrawQuad();
    DrawGui(window);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  DOGL(glDeleteProgram(program));
  return 0;
}

