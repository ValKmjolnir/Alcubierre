#include "ui/widget/checkbox.hpp"

void checkbox::draw() {
    auto checked = (game_config::singleton().*get)();

    Rectangle rec = { (float)x, (float)y, (float)size, (float)size };
    const char* text = checked? "o" : " ";
    auto final_color = color;
    if (mouse_pressed) {
        final_color = ColorAlpha(color, 0.5f);
    }
    DrawRectangleRounded(rec, 0.5f, 15, final_color);
    DrawText(text, x + size / 2 - MeasureText(text, 20) / 2, y + size / 2 - 10, 20, BLACK);
}

void checkbox::mouse_release_call_back() {
    auto checked = (game_config::singleton().*get)();
    (game_config::singleton().*set)(!checked);
}
