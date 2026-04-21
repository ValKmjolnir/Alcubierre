#pragma once

#include <raylib.h>
#include "window.hpp"

class menu {
private:
    game_window& window;
    bool flag_show_mouse = true;

public:
    menu(game_window& window) : window(window) {}
    void draw();
    void hide_mouse() {
        if (flag_show_mouse) {
            flag_show_mouse = false;
            DisableCursor();
        }
    }
    void show_mouse() {
        if (!flag_show_mouse) {
            flag_show_mouse = true;
            EnableCursor();
        }
    }
};