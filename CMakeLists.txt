set(DPI 200)
set(CL 1) # 0=off, 1=on

cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE True)
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra")

set(ME emergence)
project($ME)

include_directories(${PROJECT_SOURCE_DIR}/opt)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
set(glm_DIR ${PROJECT_SOURCE_DIR}/opt/glm) # voidlinux workaround
find_package(glm REQUIRED)
find_package(OpenCL)
find_package(OpenGL REQUIRED)

add_library(imgui STATIC
  opt/imgui/imgui.cpp
  opt/imgui/imgui_demo.cpp
  opt/imgui/imgui_draw.cpp
  opt/imgui/imgui_impl_glfw.cpp
  opt/imgui/imgui_impl_opengl3.cpp
  opt/imgui/imgui_widgets.cpp)

add_library(lib${ME} STATIC
  # core
  src/state/state.cc
  src/proc/cl.cc
  src/proc/proc.cc
  src/proc/control.cc
  # view
  src/view/view.cc
  src/view/headless.cc
  src/view/gl.cc
  src/view/gui.cc
  src/view/canvas.cc
  # util
  src/util/util.cc
  src/util/log.cc)

target_compile_definitions(lib${ME} PUBLIC DPI=${DPI})
target_compile_definitions(lib${ME} PUBLIC MESA_GL_VERSION_OVERRIDE=3.3)
target_compile_definitions(lib${ME} PUBLIC MESA_GLSL_VERSION_OVERRIDE=330)

if(OpenCL_FOUND AND EXISTS "${OpenCL_INCLUDE_DIR}/CL/cl2.hpp")
  target_compile_definitions(lib${ME} PUBLIC HAS_CL=${CL})
  target_compile_definitions(lib${ME} PUBLIC CL_TARGET_OPENCL_VERSION=210)
  target_compile_definitions(lib${ME} PUBLIC CL_HPP_TARGET_OPENCL_VERSION=210)
  set(LIBRARIES lib${ME} GLEW glfw imgui OpenCL OpenGL)
else()
  set(LIBRARIES lib${ME} GLEW glfw imgui OpenGL)
endif()

target_link_libraries(lib${ME} imgui)

add_executable(${ME} src/main.cc)
enable_testing()
add_executable(test${ME} test/test.cc)

target_link_libraries(${ME} ${LIBRARIES})
target_link_libraries(test${ME} ${LIBRARIES})

