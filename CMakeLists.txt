cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(emergence)

include_directories(${PROJECT_SOURCE_DIR}/opt)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
include_directories(${GLM_INCLUDE_DIR})
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui OBJECT
  opt/imgui/imgui.cpp
  opt/imgui/imgui_demo.cpp
  opt/imgui/imgui_draw.cpp
  opt/imgui/imgui_impl_glfw.cpp
  opt/imgui/imgui_impl_opengl3.cpp
  opt/imgui/imgui_widgets.cpp)

add_library(libemergence OBJECT
  # core
  src/proc/proc.cc
  src/proc/cl.cc
  src/state/state.cc
  # view
  src/view/view.cc
  src/view/headless.cc
  src/view/canvas.cc
  src/view/gui.cc
  src/view/gl.cc
  # util
  src/util/io.cc
  src/util/log.cc
  src/util/math.cc
  src/util/observation.cc
  src/util/string.cc)
target_link_libraries(libemergence
  imgui)

add_executable(emergence src/main.cc)
target_link_libraries(emergence libemergence
  imgui GLEW glfw OpenCL OpenGL)

enable_testing()
add_executable(testemergence test/test.cc)
target_link_libraries(testemergence libemergence
  imgui GLEW glfw OpenCL OpenGL)

