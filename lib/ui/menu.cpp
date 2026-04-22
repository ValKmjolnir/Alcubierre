#include "ui/menu.hpp"

void menu::draw() {
    if (!show_menu_) {
        return;
    }
    DrawRectangle(0, 0, window.width() / 2, window.height(), { 0, 0, 0, 150 });
    DrawText("Press M to close menu", 10, 10, 20, WHITE);
}

void menu::hide_mouse() {
    if (show_mouse_) {
        show_mouse_ = false;
        DisableCursor();
    }
}

void menu::show_mouse() {
    if (!show_mouse_) {
        show_mouse_ = true;
        EnableCursor();
    }
}
