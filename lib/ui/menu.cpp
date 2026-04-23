#include <raylib.h>

#include "ui/menu.hpp"

void menu::add_new_button(int x, int y, int width, int height, const char* text, button::callback_t cb) {
    buttons.emplace_back(
        x, y,
        width, height,
        Color { 0x00, 0xdd, 0xff, 175 },
        text,
        cb
    );
}

void menu::draw() {
    if (!show_menu_) {
        return;
    }
    DrawRectangle(0, 0, int(window.width() * 0.62), window.height(), { 0, 0, 0, 150 });
    DrawText("Press ESC to close menu", 10, 10, 20, WHITE);

    for (auto& button : buttons) {
        button.draw();
    }
}
