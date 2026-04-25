#pragma once

#include <vector>

#include "utils/game_config.hpp"
#include "ui/button.hpp"
#include "window.hpp"

class menu {
private:
    game_window& window;
    std::vector<button> buttons;
    bool show_menu_ = false;

public:
    menu(game_window& window) : window(window) {}
    void add_new_button(int x,
                        int y,
                        int width,
                        int height,
                        button_operation op,
                        const char* text,
                        config_set_funcptr cb);
    void draw();
    void set_show_menu(bool flag) { show_menu_ = flag; }
    bool get_show_menu() const { return show_menu_; }
};
