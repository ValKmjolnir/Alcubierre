#include <raylib.h>

#include "ui/mouse_cursor.hpp"

namespace {

constexpr float kInner = 4.0f;
constexpr float kOuter = 9.0f;
constexpr float kCoreRadius = 1.4f;

const Color kShadow { 0, 0, 0, 150 };
const Color kArm { 140, 215, 255, 235 };
const Color kCenter { 235, 244, 255, 255 };

void draw_cross(Vector2 center, float thickness, Color color) {
    DrawLineEx({ center.x - kOuter, center.y },
               { center.x - kInner, center.y },
               thickness, color);
    DrawLineEx({ center.x + kInner, center.y },
               { center.x + kOuter, center.y },
               thickness, color);
    DrawLineEx({ center.x, center.y - kOuter },
               { center.x, center.y - kInner },
               thickness, color);
    DrawLineEx({ center.x, center.y + kInner },
               { center.x, center.y + kOuter },
               thickness, color);
}

}

void mouse_cursor::draw() const {
    const Vector2 pos = GetMousePosition();
    const Vector2 shadow { pos.x + 1.0f, pos.y + 1.0f };

    draw_cross(shadow, 2.5f, kShadow);
    DrawCircleV(shadow, kCoreRadius + 0.6f, kShadow);

    draw_cross(pos, 1.5f, kArm);
    DrawCircleV(pos, kCoreRadius, kCenter);
}
