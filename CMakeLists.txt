cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(emergence)

add_executable(emergence src/main.cc
                         src/util/debug.cc
                         src/util/file.cc
                         src/util/string.cc
                         src/gl/ibuffer.cc
                         src/gl/varray.cc
                         src/gl/vbuffer.cc
                         src/gl/vbufferlayout.cc
                         src/gl/shader.cc
                         src/processor/processor.cc
                         src/state/state.cc
                         src/view/view.cc
                         src/view/headless.cc
                         src/view/visualiser.cc
                         src/view/drawer.cc
                         src/view/gui.cc
                         opt/imgui.cpp
                         opt/imgui_demo.cpp
                         opt/imgui_draw.cpp
                         opt/imgui_impl_glfw.cpp
                         opt/imgui_impl_opengl3.cpp
                         opt/imgui_widgets.cpp)
include_directories(opt)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
target_link_libraries(emergence GLEW glfw OpenCL OpenGL)

