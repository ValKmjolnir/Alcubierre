#include <raylib.h>
#include <raymath.h>

#include <cstdio>

#include "ui/warp_anchor.hpp"
#include "camera.hpp"

namespace {

constexpr float kRadius = 10.0f;
constexpr float kThickness = 1.6f;
constexpr float kHitRadius = 14.0f;
constexpr double kKmPerAu = 149597870.7;

const Color kShadow { 0, 0, 0, 160 };
const Color kColor { 0, 221, 255, 220 };
const Color kCore { 235, 244, 255, 235 };

bool anchor_screen_position(const camera_3d& camera, int width, int height,
                            const Vector3& position, Vector2* screen) {
    const Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target(), camera.position()));
    const Vector3 to_anchor = Vector3Subtract(position, camera.position());
    if (Vector3DotProduct(to_anchor, forward) <= 0.0f) {
        return false;
    }

    const Vector2 projected = GetWorldToScreenEx(
        position, camera.get_camera(), width, height);
    if (projected.x < 0.0f || projected.x > float(width)
        || projected.y < 0.0f || projected.y > float(height)) {
        return false;
    }

    *screen = projected;
    return true;
}

void draw_diamond(Vector2 center, Color color) {
    DrawLineEx({ center.x, center.y - kRadius },
               { center.x + kRadius, center.y }, kThickness, color);
    DrawLineEx({ center.x + kRadius, center.y },
               { center.x, center.y + kRadius }, kThickness, color);
    DrawLineEx({ center.x, center.y + kRadius },
               { center.x - kRadius, center.y }, kThickness, color);
    DrawLineEx({ center.x - kRadius, center.y },
               { center.x, center.y - kRadius }, kThickness, color);
}

}

warp_anchor::warp_anchor(Vector3 position, const char* name)
    : position_(position)
    , name_(name) {}

void warp_anchor::draw(const camera_3d& camera, int width, int height,
                       const Vector3& player_position) const {
    Vector2 screen;
    if (!anchor_screen_position(camera, width, height, position_, &screen)) {
        return;
    }

    draw_diamond({ screen.x + 1.0f, screen.y + 1.0f }, kShadow);
    draw_diamond(screen, kColor);
    DrawCircleLines(int(screen.x), int(screen.y), 4.0f, kColor);
    DrawCircleV(screen, 1.2f, kCore);

    const float distance_km = Vector3Distance(player_position, position_) * 0.01f;
    const double distance_au = double(distance_km) / kKmPerAu;

    char label[96];
    if (distance_au >= 0.01) {
        snprintf(label, sizeof(label), "%s  %.2f AU", name_, distance_au);
    } else {
        snprintf(label, sizeof(label), "%s  %.1f km", name_, distance_km);
    }

    const int font_size = 12;
    const int text_width = MeasureText(label, font_size);
    const int x = int(screen.x) - text_width / 2;
    const int y = int(screen.y + kRadius + 8.0f);
    DrawText(label, x + 1, y + 1, font_size, kShadow);
    DrawText(label, x, y, font_size, kColor);
}

bool warp_anchor::hit_test(const camera_3d& camera, int width, int height,
                           Vector2 point) const {
    Vector2 screen;
    if (!anchor_screen_position(camera, width, height, position_, &screen)) {
        return false;
    }
    return Vector2Distance(point, screen) <= kHitRadius;
}
