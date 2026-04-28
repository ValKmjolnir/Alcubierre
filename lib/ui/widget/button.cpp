#include "utils/game_config.hpp"
#include "ui/widget/button.hpp"

void button::draw() {
    Rectangle rec = { (float)x, (float)y, (float)width, (float)height };
    
    auto final_color = color;
    if (mouse_pressed) {
        final_color = ColorAlpha(color, 0.5f);
    }

    DrawRectangleRounded(rec, 0.5f, 15, final_color);
    DrawText(text.c_str(), x + width / 2 - MeasureText(text.c_str(), 20) / 2, y + height / 2 - 10, 20, BLACK);
}

void button::mouse_release_call_back() {
    if (!callback) {
        return;
    }
    if (operation == button_operation::TOGGLE) {
        flag = !flag;
        (game_config::singleton().*callback)(flag);
    } else if (operation == button_operation::SET_TRUE) {
        (game_config::singleton().*callback)(true);
    } else if (operation == button_operation::SET_FALSE) {
        (game_config::singleton().*callback)(false);
    }
}