#include "ui/widget/selector.hpp"

void selector::draw_text_centered(Rectangle& target,
                                  const char* text,
                                  int fontsize,
                                  Color color) {
    auto middle_pos = target.x + target.width / 2.0f;
    auto string_width = MeasureText(text, fontsize);
    DrawText(text, middle_pos - string_width / 2.0f, target.y + target.height / 2 - 5, fontsize, color);
}

void selector::draw() {
    Rectangle left = { float(x), float(y), float(width / 8) - 2.5f, float(height) };
    Rectangle right = { float(x + width / 8 * 7 + 5) - 2.5f, float(y), float(width / 8) - 2.5f, float(height) };
    DrawRectangleRounded(left, 0.5f, 16, color);
    DrawRectangleRounded(right, 0.5f, 16, color);
    draw_text_centered(left, "<", 16, GRAY);
    draw_text_centered(right, ">", 16, GRAY);

    Rectangle middle = { float(x + width / 8), float(y), float(width / 8 * 6), float(height) };
    const auto middle_color = ColorAlpha(color, 0.3f);
    DrawRectangleRounded(middle, 0.5f, 16, middle_color);
    draw_text_centered(middle, "1600x800", 16, GRAY);
}
