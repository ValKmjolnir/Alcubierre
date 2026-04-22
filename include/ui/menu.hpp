#pragma once

#include <raylib.h>
#include "window.hpp"

class menu {
private:
    game_window& window;
    bool show_mouse_ = true;

public:
    menu(game_window& window) : window(window) {}
    void draw();
    void hide_mouse() {
        if (show_mouse_) {
            show_mouse_ = false;
            DisableCursor();
        }
    }
    void show_mouse() {
        if (!show_mouse_) {
            show_mouse_ = true;
            EnableCursor();
        }
    }
};
