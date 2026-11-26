#include <cstdio>

#include "ui/widget/selector.hpp"
#include "window.hpp"

Rectangle selector::arrow_left() const {
    return { float(x), float(y), float(width / 8) - 2.5f, float(height) };
}

Rectangle selector::arrow_right() const {
    return { float(x + width / 8 * 7) + 2.5f, float(y), float(width / 8) - 2.5f, float(height) };
}

Rectangle selector::middle() const {
    return { float(x + width / 8), float(y), float(width / 8 * 6), float(height) };
}

int selector::current_index() const {
    if (!window || !options) {
        return -1;
    }
    for (int i = 0; i < option_count; i++) {
        if (options[i].width == window->width() &&
            options[i].height == window->height()) {
            return i;
        }
    }
    return -1;
}

bool selector::is_hovered(int mouse_x, int mouse_y) const {
    const Vector2 pos { float(mouse_x), float(mouse_y) };
    const Rectangle rec { float(x), float(y), float(width), float(height) };
    return CheckCollisionPointRec(pos, rec);
}

void selector::mouse_press_call_back() {
    const Vector2 pos = GetMousePosition();
    if (CheckCollisionPointRec(pos, arrow_left())) {
        pressed_side = -1;
    } else if (CheckCollisionPointRec(pos, arrow_right())) {
        pressed_side = 1;
    } else {
        pressed_side = 0;
    }
}

void selector::mouse_release_call_back() {
    if (!window || !options || pressed_side == 0) {
        pressed_side = 0;
        return;
    }

    const int index = current_index();
    const int next = index + pressed_side;
    pressed_side = 0;

    if (index < 0 || next < 0 || next >= option_count) {
        return;
    }

    window->set_resolution(options[next].width, options[next].height);
}

void selector::move(int dx, int dy) {
    x += dx;
    y += dy;
}

void selector::draw_text_centered(const Rectangle& target,
                                  const char* text,
                                  int fontsize,
                                  Color color) {
    const auto middle_pos = target.x + target.width / 2.0f;
    auto string_width = MeasureText(text, fontsize);
    DrawText(text, middle_pos - string_width / 2.0f, target.y + target.height / 2 - 5, fontsize, color);
}

void selector::draw() {
    const int index = current_index();

    Color left_color = color;
    Color right_color = color;
    if (index <= 0) {
        left_color = ColorAlpha(color, 0.3f);
    }
    if (index < 0 || index >= option_count - 1) {
        right_color = ColorAlpha(color, 0.3f);
    }

    const Rectangle left = arrow_left();
    const Rectangle right = arrow_right();
    const Rectangle center = middle();

    DrawRectangleRounded(left, 0.5f, 16, left_color);
    DrawRectangleRounded(right, 0.5f, 16, right_color);
    draw_text_centered(left, "<", 16, GRAY);
    draw_text_centered(right, ">", 16, GRAY);

    const auto middle_color = ColorAlpha(color, 0.3f);
    DrawRectangleRounded(center, 0.5f, 16, middle_color);

    char actual[32];
    const char* label = "n/a";
    if (index >= 0) {
        label = options[index].label;
    } else if (window) {
        snprintf(actual, sizeof(actual), "%dx%d", window->width(), window->height());
        label = actual;
    }
    draw_text_centered(center, label, 16, GRAY);
}
