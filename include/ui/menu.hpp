#pragma once

#include "ui/ui_window.hpp"

class game_window;

class menu {
private:
    game_window& window;
    ui_window panel;
    bool show_menu_ = false;

public:
    menu(game_window& window);
    void add_new_button(int x,
                        int y,
                        int width,
                        int height,
                        button_operation op,
                        const char* text,
                        config_set_funcptr cb);
    void add_new_checkbox(int x,
                          int y,
                          int size,
                          config_get_funcptr gf,
                          config_set_funcptr cf);
    void add_new_slider(int x,
                        int y,
                        int width,
                        int height,
                        int min_value,
                        int max_value,
                        config_get_int_funcptr gf,
                        config_set_int_funcptr cf);
    void add_new_textbox(int x,
                         int y,
                         int width,
                         int height,
                         int fontsize,
                         Color color,
                         const char* text);
    void add_new_selector(int x,
                          int y,
                          int width,
                          int height,
                          const resolution_option* options,
                          int option_count);

    void check_mouse_press();
    void draw(bool accept_input = true);
    void set_show_menu(bool flag);
    bool get_show_menu() const { return show_menu_; }
};
