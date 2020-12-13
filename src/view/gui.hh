#pragma once

#include "canvas.hh"
#include "../proc/control.hh"
#include "../util/log.hh"
#include "../util/util.hh"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


// GuiState: Intermediary state storage before really modifying State

class GuiState
{
  public:
    GuiState(Control& ctrl);
    bool untrue();
    bool deceive();
    void random(Log& log);
    void preset(Log& log);
    bool save(const std::string& path);
    bool load(const std::string& path);

    long long    stop_;
    int          num_;
    unsigned int width_;
    unsigned int height_;
    float        alpha_;
    float        beta_;
    float        scope_;
    float        speed_;
    int          colors_;
    int          preset_;

  private:
    Control& ctrl_;
};


// Gui: The part of the visualisation dealing with the user interface.

class Canvas;

class Gui
{
  public:
    Canvas&     canvas_;
    GLFWwindow* view_;

    Gui(Log& log, GuiState state, Canvas& canvas,
        unsigned int width, unsigned int height, bool hide_side);
    ~Gui();
    void draw();
    void draw_side(bool draw);
    void draw_console(bool draw);
    void draw_save_load(char dialog);
    void draw_quit(char dialog);
    void next() const;
    void pause();
    void set_pointer();
    void close();
    bool closing() const;
    static void key_callback(GLFWwindow* view, int key, int scancode,
                             int action, int mods);
    static void mouse_button_callback(GLFWwindow* view, int button, int action,
                                      int mods);
    static void mouse_move_callback(GLFWwindow* view, double x, double y);
    static void mouse_scroll_callback(GLFWwindow* view, double dx, double dy);
    static void resize_callback(GLFWwindow* view, int w, int h);

  private:
    GuiState     state_;
    Log&         log_;
    ImFont*      font_r;
    ImFont*      font_b;
    ImFont*      font_i;
    ImFont*      font_z;
    int          font_width_;
    unsigned int gui_width_;
    unsigned int gui_height_;
    bool         side_;
    bool         console_;
    char         dialog_;
    double       ago_;
    unsigned int frames_;
    float        fps_;
    double       x_;
    double       y_;
    bool         dolly_;
    bool         pivot_;
};

