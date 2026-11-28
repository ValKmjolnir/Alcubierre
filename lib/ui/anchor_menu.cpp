#include <raylib.h>
#include <raymath.h>

#include "ui/anchor_menu.hpp"

namespace {

constexpr int kOptionCount = 2;
constexpr float kWidth = 180.0f;
constexpr float kRowHeight = 30.0f;
constexpr float kPadding = 4.0f;
constexpr int kFontSize = 13;

const char* kLabels[] = { "ALIGN TO", "WARP TO" };

const Color kBackground { 8, 12, 18, 240 };
const Color kBorder { 0, 221, 255, 120 };
const Color kHover { 20, 30, 42, 240 };
const Color kText { 200, 210, 220, 255 };
const Color kShadow { 0, 0, 0, 120 };

int option_at(const Rectangle& bounds, Vector2 point) {
    if (point.x < bounds.x || point.x > bounds.x + bounds.width) {
        return -1;
    }
    for (int i = 0; i < kOptionCount; i++) {
        const float top = bounds.y + kPadding + i * kRowHeight;
        if (point.y >= top && point.y <= top + kRowHeight) {
            return i;
        }
    }
    return -1;
}

}

void anchor_menu::open(Vector2 mouse, int width, int height) {
    const float menu_height = kOptionCount * kRowHeight + 2.0f * kPadding;
    float x = mouse.x + 12.0f;
    float y = mouse.y + 12.0f;

    if (x + kWidth > float(width) - 8.0f) {
        x = mouse.x - kWidth - 12.0f;
    }
    if (y + menu_height > float(height) - 8.0f) {
        y = mouse.y - menu_height - 12.0f;
    }

    bounds_ = {
        Clamp(x, 8.0f, float(width) - kWidth - 8.0f),
        Clamp(y, 8.0f, float(height) - menu_height - 8.0f),
        kWidth,
        menu_height
    };
    visible_ = true;
    pressed_ = -1;
}

void anchor_menu::close() {
    visible_ = false;
    pressed_ = -1;
}

bool anchor_menu::visible() const {
    return visible_;
}

anchor_action anchor_menu::update() {
    if (!visible_) {
        return anchor_action::none;
    }

    const Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (!CheckCollisionPointRec(mouse, bounds_)) {
            close();
            return anchor_action::none;
        }
        pressed_ = option_at(bounds_, mouse);
        return anchor_action::none;
    }

    if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        return anchor_action::none;
    }

    const int released = option_at(bounds_, mouse);
    const int pressed = pressed_;
    pressed_ = -1;

    if (pressed < 0) {
        return anchor_action::none;
    }

    close();
    if (released != pressed) {
        return anchor_action::none;
    }
    return pressed == 0 ? anchor_action::align_to : anchor_action::warp_to;
}

void anchor_menu::draw() const {
    if (!visible_) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    const int hovered = option_at(bounds_, mouse);

    DrawRectangle(int(bounds_.x) + 2, int(bounds_.y) + 2,
                  int(bounds_.width), int(bounds_.height), kShadow);
    DrawRectangleRec(bounds_, kBackground);
    DrawRectangleLinesEx(bounds_, 1.0f, kBorder);

    for (int i = 0; i < kOptionCount; i++) {
        const Rectangle row {
            bounds_.x + 1.0f,
            bounds_.y + kPadding + i * kRowHeight,
            bounds_.width - 2.0f,
            kRowHeight
        };
        if (i == hovered) {
            DrawRectangleRec(row, kHover);
        }

        const int text_width = MeasureText(kLabels[i], kFontSize);
        DrawText(kLabels[i],
                 int(row.x + (row.width - text_width) * 0.5f),
                 int(row.y + (row.height - kFontSize) * 0.5f),
                 kFontSize,
                 i == hovered ? WHITE : kText);
    }

    const float divider_y = bounds_.y + kPadding + kRowHeight;
    DrawLineEx({ bounds_.x + 1.0f, divider_y },
               { bounds_.x + bounds_.width - 1.0f, divider_y },
               1.0f, Color { 0, 221, 255, 50 });
}
