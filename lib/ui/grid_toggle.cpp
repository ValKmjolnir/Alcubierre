#include <raylib.h>

#include "ui/grid_toggle.hpp"
#include "utils/game_config.hpp"

namespace {

constexpr float kSize = 64.0f;
constexpr float kGap = 8.0f;

const Color kOn { 0x00, 0xdd, 0xff, 255 };
const Color kOff { 110, 120, 132, 255 };

}

Rectangle grid_toggle::button_rect(const Rectangle& status_bounds) const {
    return { status_bounds.x - kGap - kSize, status_bounds.y, kSize, kSize };
}

bool grid_toggle::contains(const Rectangle& status_bounds, int x, int y) const {
    return CheckCollisionPointRec(
        Vector2 { float(x), float(y) }, button_rect(status_bounds));
}

void grid_toggle::draw(const Rectangle& status_bounds) const {
    const Rectangle rect = button_rect(status_bounds);
    const bool enabled = game_config::singleton().get_enable_grid_draw();
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    const Color accent = enabled ? kOn : kOff;

    DrawRectangleRec(rect, hovered ? Color { 16, 22, 32, 230 } : Color { 10, 14, 20, 205 });
    DrawRectangleLinesEx(rect, 1.0f,
                         enabled ? Color { 0x00, 0xdd, 0xff, 180 } : Color { 0x00, 0xdd, 0xff, 60 });

    const char* title = "GRID";
    const char* state = enabled ? "ON" : "OFF";

    const int title_width = MeasureText(title, 11);
    DrawText(title,
             int(rect.x + (rect.width - title_width) * 0.5f),
             int(rect.y) + 13,
             11,
             accent);

    const int state_width = MeasureText(state, 14);
    DrawText(state,
             int(rect.x + (rect.width - state_width) * 0.5f),
             int(rect.y) + 34,
             14,
             accent);
}
