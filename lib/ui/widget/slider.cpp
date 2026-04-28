#include <raylib.h>
#include <raymath.h>
#include <cstdio>

#include "ui/widget/slider.hpp"

void slider::draw() {
    int real_value = (game_config::singleton().*get)();
    ratio = (real_value - min_value) * 1.0 / (max_value - min_value);

    const float button_size = height / 2;
    const float value_related_width = width - button_size;
    DrawLine(x, y + button_size, x + width, y + button_size, GRAY);

    if (mouse_pressed) {
        auto pos = GetMousePosition();
        ratio = (pos.x - x) / value_related_width;
        ratio = Clamp(ratio, 0.0f, 1.0f);
    }

    Rectangle rec = {
        x + value_related_width * ratio,
        y + button_size / 2,
        button_size,
        button_size
    };

    if (mouse_pressed) {
        Rectangle rec_edge = {
            static_cast<float>(x - 1),
            static_cast<float>(y - 1),
            static_cast<float>(width + 1),
            static_cast<float>(height + 1)
        };
        DrawRectangleRoundedLines(rec_edge, 0.5f, 32, WHITE);
    }
    DrawRectangleRounded(rec, 0.5f, 15, color);

    char ratio_buff[32];
    snprintf(ratio_buff, 31, "%d", int(ratio * (max_value - min_value) + min_value));
    DrawText(ratio_buff, x + width + 10, y + 4, 16, WHITE);
}

void slider::mouse_release_call_back() {
    int real_value = static_cast<int>(ratio * (max_value - min_value) + min_value);
    (game_config::singleton().*set)(real_value);
}