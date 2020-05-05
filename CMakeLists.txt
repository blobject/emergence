cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(emergence)

# imgui
include_directories(${PROJECT_SOURCE_DIR}/opt)

# opencl
#include_directories(/opt/intel/include)
#include_directories(${PROJECT_SOURCE_DIR}/opt/opencl)
#link_directories(/opt/intel/lib /opt/intel/lib64)

add_library(imgui OBJECT
  opt/imgui/imgui.cpp
  opt/imgui/imgui_demo.cpp
  opt/imgui/imgui_draw.cpp
  opt/imgui/imgui_impl_glfw.cpp
  opt/imgui/imgui_impl_opengl3.cpp
  opt/imgui/imgui_widgets.cpp)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
include_directories(${GLM_INCLUDE_DIR})
find_package(OpenGL REQUIRED)

add_executable(emergence src/main.cc
  # core
  src/sys/sys.cc
  src/state/state.cc
  src/view/view.cc
  src/view/headless.cc
  src/view/visualiser.cc
  src/view/gui.cc
  # util
  src/util/debug.cc
  src/util/io.cc
  src/util/math.cc
  src/util/random.cc
  src/util/string.cc
  # gl
  src/gl/indexbuffer.cc
  src/gl/vertexarray.cc
  src/gl/vertexbuffer.cc
  src/gl/vertexbufferlayout.cc
  src/gl/shader.cc)

target_link_libraries(emergence imgui GLEW glfw OpenGL)

