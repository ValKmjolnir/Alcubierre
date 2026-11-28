#include <raylib.h>
#include <raymath.h>

#include "ui/ship_brackets.hpp"
#include "camera.hpp"
#include "ship/ship.hpp"

namespace {

constexpr float kHalfSize = 8.0f;
constexpr float kArm = 4.0f;
constexpr float kThickness = 1.2f;

const Color kShadow { 0, 0, 0, 110 };
const Color kBracket { 140, 215, 255, 150 };

void draw_corners(Vector2 center, Color color) {
    const float left = center.x - kHalfSize;
    const float right = center.x + kHalfSize;
    const float top = center.y - kHalfSize;
    const float bottom = center.y + kHalfSize;

    DrawLineEx({ left, top }, { left + kArm, top }, kThickness, color);
    DrawLineEx({ left, top }, { left, top + kArm }, kThickness, color);

    DrawLineEx({ right - kArm, top }, { right, top }, kThickness, color);
    DrawLineEx({ right, top }, { right, top + kArm }, kThickness, color);

    DrawLineEx({ left, bottom - kArm }, { left, bottom }, kThickness, color);
    DrawLineEx({ left, bottom }, { left + kArm, bottom }, kThickness, color);

    DrawLineEx({ right, bottom - kArm }, { right, bottom }, kThickness, color);
    DrawLineEx({ right - kArm, bottom }, { right, bottom }, kThickness, color);
}

}

void ship_brackets::draw(const camera_3d& camera, int width, int height,
                         const std::vector<ship*>& ships,
                         const ship* target) const {
    const Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target(), camera.position()));

    for (const ship* candidate : ships) {
        if (!candidate || candidate->destroyed() || candidate == target) {
            continue;
        }

        const Vector3 center = candidate->aim_point();
        const Vector3 to_ship = Vector3Subtract(center, camera.position());
        if (Vector3DotProduct(to_ship, forward) <= 0.0f) {
            continue;
        }

        const Vector2 screen = GetWorldToScreenEx(
            center, camera.get_camera(), width, height);
        if (screen.x < 0.0f || screen.x > float(width)
            || screen.y < 0.0f || screen.y > float(height)) {
            continue;
        }

        draw_corners({ screen.x + 1.0f, screen.y + 1.0f }, kShadow);
        draw_corners(screen, kBracket);
    }
}
