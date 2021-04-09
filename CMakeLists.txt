set(DPI 200) # higher dpi decreases font size
set(CL 1) # 0=off, 1=on

cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE True)
#set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra")

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
  src/proc/cl.cc
  src/proc/control.cc
  src/proc/proc.cc
  src/state/state.cc
  # exp
  src/exp/exp.cc
  # view
  src/view/canvas.cc
  src/view/gl.cc
  src/view/gui.cc
  src/view/image.cc
  src/view/headless.cc
  src/view/state.cc
  src/view/view.cc
  # util
  src/util/log.cc
  src/util/util.cc
)

target_compile_definitions(lib${ME} PUBLIC DPI=${DPI})
target_compile_definitions(lib${ME} PUBLIC MESA_GL_VERSION_OVERRIDE=3.3)
target_compile_definitions(lib${ME} PUBLIC MESA_GLSL_VERSION_OVERRIDE=330)
#target_link_libraries(lib${ME} imgui)

set(LIBS lib${ME} GLEW glfw imgui OpenGL)
if(OpenCL_FOUND AND EXISTS "${OpenCL_INCLUDE_DIR}/CL/cl2.hpp")
  target_compile_definitions(lib${ME} PUBLIC CL_ENABLED=${CL})
  target_compile_definitions(lib${ME} PUBLIC CL_TARGET_OPENCL_VERSION=210)
  target_compile_definitions(lib${ME} PUBLIC CL_HPP_TARGET_OPENCL_VERSION=210)
  set(LIBS ${LIBS} OpenCL)
endif()

add_executable(${ME} src/main.cc)
enable_testing()
add_executable(test${ME} test/test.cc)

target_link_libraries(${ME} ${LIBS})
target_link_libraries(test${ME} ${LIBS})

