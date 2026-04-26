#include <raylib.h>

#include "ui/menu.hpp"

void menu::add_new_button(int x,
                          int y,
                          int width,
                          int height,
                          button_operation op,
                          const char* text,
                          config_set_funcptr cb) {
    buttons.emplace_back(
        x, y,
        width, height,
        Color { 0x00, 0xdd, 0xff, 175 },
        op,
        text,
        cb
    );
}

void menu::add_new_checkbox(int x,
                            int y,
                            int size,
                            config_get_funcptr gf,
                            config_set_funcptr cf) {
    checkboxes.emplace_back(x, y, size, Color { 0x00, 0xdd, 0xff, 200 }, gf, cf);
}

void menu::add_new_slider(int x, int y, int width, int height) {
    sliders.emplace_back(x, y, width, height, Color { 0x00, 0xdd, 0xff, 200 });
}

void menu::add_new_textbox(int x,
                           int y,
                           int width,
                           int height,
                           int fontsize,
                           Color color,
                           const char* text) {
    textboxes.emplace_back(x, y, width, height, fontsize, color, text);
}

void menu::draw() {
    if (!show_menu_) {
        return;
    }
    DrawRectangle(0, 0, int(window.width() * 0.31), window.height(), { 0, 0, 0, 150 });

    for (auto& button : buttons) {
        button.draw();
    }
    for (auto& checkbox : checkboxes) {
        checkbox.draw();
    }
    for (auto& slider : sliders) {
        slider.draw();
    }
    for (auto& textbox : textboxes) {
        textbox.draw();
    }
}
