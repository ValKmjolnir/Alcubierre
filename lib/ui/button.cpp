#include "ui/button.hpp"

void button::draw() {
    Rectangle rec = { (float)x, (float)y, (float)width, (float)height };
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        auto pos = GetMousePosition();
        if (is_hovered(pos.x, pos.y)) {
            auto hover_color = ColorAlpha(color, 0.5f);
            DrawRectangleRounded(rec, 0.5f, 15, hover_color);
            DrawText(text.c_str(), x + width / 2 - MeasureText(text.c_str(), 20) / 2, y + height / 2 - 10, 20, BLACK);

            if (callback != nullptr) {
                callback();
            }
            return;
        }
    }

    DrawRectangleRounded(rec, 0.5f, 15, color);
    DrawText(text.c_str(), x + width / 2 - MeasureText(text.c_str(), 20) / 2, y + height / 2 - 10, 20, BLACK);
}