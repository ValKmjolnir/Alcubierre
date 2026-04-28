#pragma once

#include <vector>

#include "utils/game_config.hpp"
#include "ui/widget/widget.hpp"
#include "ui/widget/button.hpp"
#include "ui/widget/checkbox.hpp"
#include "ui/widget/selector.hpp"
#include "ui/widget/slider.hpp"
#include "ui/widget/textbox.hpp"
#include "window.hpp"

class menu {
private:
    game_window& window;
    std::vector<button> buttons;
    std::vector<checkbox> checkboxes;
    std::vector<selector> selectors;
    std::vector<slider> sliders;
    std::vector<textbox> textboxes;
    bool show_menu_ = false;

    widget* hovered_widget = nullptr;

public:
    menu(game_window& window) : window(window) {}
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
                          int height);

    void check_mouse_press();
    void draw();
    void set_show_menu(bool flag) { show_menu_ = flag; }
    bool get_show_menu() const { return show_menu_; }
};
