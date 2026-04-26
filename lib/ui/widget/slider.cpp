#include <raylib.h>
#include <raymath.h>
#include <cstdio>

#include "ui/widget/slider.hpp"

void slider::draw() {
    const float button_size = height / 2;
    const float value_related_width = width - button_size;
    DrawLine(x, y + button_size, x + width, y + button_size, GRAY);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        auto pos = GetMousePosition();
        if (is_hovered(pos.x, pos.y) || hovered) {
            value = (pos.x - x) / value_related_width;
            value = Clamp(value, 0.0f, 1.0f);
            hovered = true;
        }
    } else {
        hovered = false;
    }

    Rectangle rec = {
        x + value_related_width * value,
        y + button_size / 2,
        button_size,
        button_size
    };

    if (hovered) {
        Rectangle rec_edge = {
            static_cast<float>(x - 1),
            static_cast<float>(y - 1),
            static_cast<float>(width + 1),
            static_cast<float>(height + 1)
        };
        DrawRectangleRoundedLines(rec_edge, 0.5f, 32, WHITE);
    }
    DrawRectangleRounded(rec, 0.5f, 15, color);

    char ratio[32];
    snprintf(ratio, 31, "%d", int(value * 100));
    DrawText(ratio, x + width + 10, y + 4, 16, WHITE);
}
