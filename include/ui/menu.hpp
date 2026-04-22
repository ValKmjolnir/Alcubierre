#pragma once

#include <raylib.h>
#include "window.hpp"

class menu {
private:
    game_window& window;
    bool show_menu_ = false;
    bool show_mouse_ = false;

public:
    menu(game_window& window) : window(window) {
        DisableCursor();
    }
    void draw();
    void hide_mouse();
    void show_mouse();
    void set_show_menu(bool flag) { show_menu_ = flag; }
    bool get_show_menu() const { return show_menu_; }
};
