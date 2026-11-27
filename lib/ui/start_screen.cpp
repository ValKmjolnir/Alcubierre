#include <raylib.h>

#include "ui/start_screen.hpp"
#include "window.hpp"

namespace {

constexpr int kButtonCount = 3;
constexpr int kButtonWidth = 280;
constexpr int kButtonHeight = 42;
constexpr int kButtonGap = 14;

const char* button_label(int index) {
    switch (index) {
    case 0: return "START GAME";
    case 1: return "SETTINGS";
    default: return "QUIT";
    }
}

int clamp_int(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

void draw_centered(const char* text, int screen_width, int y, int font_size, Color color) {
    const int text_width = MeasureText(text, font_size);
    DrawText(text,
             (screen_width - text_width) / 2,
             y,
             font_size,
             color);
}

}

start_screen::start_screen(game_window& window) : window_(window) {}

Rectangle start_screen::button_rect(int index) const {
    const int top = int(window_.height() * 0.52f);
    return {
        float((window_.width() - kButtonWidth) / 2),
        float(top + index * (kButtonHeight + kButtonGap)),
        float(kButtonWidth),
        float(kButtonHeight)
    };
}

int start_screen::hit_button(const Vector2& point) const {
    for (int i = 0; i < kButtonCount; i++) {
        if (CheckCollisionPointRec(point, button_rect(i))) {
            return i;
        }
    }
    return -1;
}

start_action start_screen::update() {
    const Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        pressed_button_ = hit_button(mouse);
    }

    if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        return start_action::none;
    }

    const int released = hit_button(mouse);
    const int pressed = pressed_button_;
    pressed_button_ = -1;

    if (pressed < 0 || released != pressed) {
        return start_action::none;
    }

    switch (pressed) {
    case 0: return start_action::start;
    case 1: return start_action::settings;
    default: return start_action::quit;
    }
}

void start_screen::draw() const {
    const int width = window_.width();
    const int height = window_.height();

    DrawRectangle(0, 0, width, height, Color { 0, 0, 0, 120 });

    const int title_size = clamp_int(int(height * 0.09f), 36, 72);
    const int title_y = int(height * 0.26f);
    const char* title = "ALCUBIERRE";
    const int title_width = MeasureText(title, title_size);
    DrawText(title,
             (width - title_width) / 2 + 2,
             title_y + 2,
             title_size,
             Color { 0, 0, 0, 170 });
    DrawText(title,
             (width - title_width) / 2,
             title_y,
             title_size,
             Color { 235, 244, 255, 255 });

    const int subtitle_size = clamp_int(int(height * 0.024f), 12, 20);
    const int subtitle_y = title_y + title_size + 6;
    draw_centered("WARP DRIVE", width, subtitle_y, subtitle_size,
                  Color { 0x00, 0xdd, 0xff, 210 });

    const int line_width = clamp_int(width / 5, 180, 360);
    DrawRectangle((width - line_width) / 2,
                  subtitle_y + subtitle_size + 14,
                  line_width,
                  1,
                  Color { 0x00, 0xdd, 0xff, 110 });

    const int label_size = clamp_int(int(height * 0.0225f), 14, 20);
    const Vector2 mouse = GetMousePosition();

    for (int i = 0; i < kButtonCount; i++) {
        const Rectangle rect = button_rect(i);
        const bool hovered = CheckCollisionPointRec(mouse, rect);
        const bool pressed = (pressed_button_ == i);

        Color background { 10, 14, 20, 215 };
        if (pressed) {
            background = Color { 24, 32, 44, 235 };
        } else if (hovered) {
            background = Color { 16, 22, 32, 230 };
        }

        DrawRectangleRec(rect, background);
        DrawRectangleLinesEx(rect, 1.0f,
                             (hovered || pressed)
                                 ? Color { 0x00, 0xdd, 0xff, 220 }
                                 : Color { 0x00, 0xdd, 0xff, 90 });

        const char* label = button_label(i);
        const int label_width = MeasureText(label, label_size);
        DrawText(label,
                 int(rect.x + (rect.width - label_width) * 0.5f),
                 int(rect.y + (rect.height - label_size) * 0.5f),
                 label_size,
                 (hovered || pressed) ? WHITE : Color { 200, 210, 220, 255 });
    }

    draw_centered("LEFT CLICK TO SELECT", width, height - 26, 10,
                  Color { 110, 120, 132, 255 });
}
