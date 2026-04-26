#include "ui/widget/checkbox.hpp"

void checkbox::draw_content(bool checked) {
    Rectangle rec = { (float)x, (float)y, (float)size, (float)size };
    const char* text = checked? "o" : " ";
    auto final_color = color;
    if (hovered) {
        final_color = ColorAlpha(color, 0.5f);
    }
    DrawRectangleRounded(rec, 0.5f, 15, final_color);
    DrawText(text, x + size / 2 - MeasureText(text, 20) / 2, y + size / 2 - 10, 20, BLACK);
}

void checkbox::draw() {
    auto checked = (game_config::singleton().*get)();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        auto pos = GetMousePosition();
        if (is_hovered(pos.x, pos.y)) {
            auto hover_color = ColorAlpha(color, 0.5f);
            hovered = true;
            draw_content(checked);

            return;
        }
        hovered = false;
    }

    if (hovered) {
        checked = !checked;
        (game_config::singleton().*set)(checked);
    }
    hovered = false;
    draw_content(checked);
}
