#include <raylib.h>
#include <raymath.h>

#include "ui/ship_brackets.hpp"
#include "camera.hpp"
#include "ship/ship.hpp"

namespace {

constexpr float kHalfSize = 8.0f;
constexpr float kArm = 4.0f;
constexpr float kThickness = 1.2f;
constexpr float kGlyphScale = 1.3f;
constexpr float kPickRadius = 9.0f;

const Color kShadow { 0, 0, 0, 110 };
const Color kBracket { 140, 215, 255, 150 };
const Color kGlyphBackdrop { 6, 12, 20, 140 };
const Color kGlyphShadow { 0, 0, 0, 180 };
const Color kGlyph { 235, 244, 255, 230 };

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

void fill_polygon(Vector2 center, const Vector2* points, int count, Color color) {
    for (int i = 1; i + 1 < count; i++) {
        DrawTriangle(
            { center.x + points[0].x * kGlyphScale, center.y + points[0].y * kGlyphScale },
            { center.x + points[i].x * kGlyphScale, center.y + points[i].y * kGlyphScale },
            { center.x + points[i + 1].x * kGlyphScale, center.y + points[i + 1].y * kGlyphScale },
            color);
    }
}

void draw_class_glyph(Vector2 center, ship_size size, Color color) {
    switch (size) {
    case ship_size::frigate: {
        const Vector2 points[] = {
            { 0.0f, -4.5f },
            { -3.2f, 3.5f },
            { 3.2f, 3.5f },
        };
        fill_polygon(center, points, 3, color);
        break;
    }
    case ship_size::cruiser: {
        const Vector2 points[] = {
            { 0.0f, -5.0f },
            { -3.5f, 0.0f },
            { 0.0f, 5.0f },
            { 3.5f, 0.0f },
        };
        fill_polygon(center, points, 4, color);
        break;
    }
    case ship_size::battleship: {
        const Vector2 points[] = {
            { -5.0f, 0.0f },
            { -3.5f, 3.0f },
            { 3.5f, 3.0f },
            { 5.0f, 0.0f },
            { 3.5f, -3.0f },
            { -3.5f, -3.0f },
        };
        fill_polygon(center, points, 6, color);
        break;
    }
    case ship_size::titan: {
        const Vector2 points[] = {
            { -5.0f, 1.0f },
            { -3.0f, 4.0f },
            { 3.0f, 4.0f },
            { 5.0f, 1.0f },
            { 2.5f, -4.5f },
            { -2.5f, -4.5f },
        };
        fill_polygon(center, points, 6, color);
        break;
    }
    }
}

void draw_glyph(Vector2 center, ship_size size) {
    DrawCircleV(center, 7.5f, kGlyphBackdrop);
    draw_class_glyph({ center.x + 1.0f, center.y + 1.0f }, size, kGlyphShadow);
    draw_class_glyph(center, size, kGlyph);
}

bool bracket_screen_position(const camera_3d& camera, int width, int height,
                             const ship& candidate, const Vector3& forward,
                             Vector2* screen) {
    const Vector3 center = candidate.aim_point();
    const Vector3 to_ship = Vector3Subtract(center, camera.position());
    if (Vector3DotProduct(to_ship, forward) <= 0.0f) {
        return false;
    }

    const Vector2 projected = GetWorldToScreenEx(
        center, camera.get_camera(), width, height);
    if (projected.x < 0.0f || projected.x > float(width)
        || projected.y < 0.0f || projected.y > float(height)) {
        return false;
    }

    *screen = projected;
    return true;
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

        Vector2 screen;
        if (!bracket_screen_position(camera, width, height, *candidate,
                                     forward, &screen)) {
            continue;
        }

        draw_corners({ screen.x + 1.0f, screen.y + 1.0f }, kShadow);
        draw_corners(screen, kBracket);
        draw_glyph(screen, candidate->ship_class().size);
    }
}

ship* ship_brackets::pick(const camera_3d& camera, int width, int height,
                          const std::vector<ship*>& ships,
                          const ship* target, Vector2 point) const {
    const Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target(), camera.position()));

    ship* best = nullptr;
    float best_screen_distance = kPickRadius;
    float best_camera_distance = 0.0f;

    for (ship* candidate : ships) {
        if (!candidate || candidate->destroyed() || candidate == target) {
            continue;
        }

        Vector2 screen;
        if (!bracket_screen_position(camera, width, height, *candidate,
                                     forward, &screen)) {
            continue;
        }

        const float screen_distance = Vector2Distance(point, screen);
        if (screen_distance > kPickRadius) {
            continue;
        }

        const float camera_distance = Vector3Distance(
            camera.position(), candidate->aim_point());
        const bool closer_on_screen = screen_distance < best_screen_distance - 0.01f;
        const bool same_on_screen = fabsf(screen_distance - best_screen_distance) <= 0.01f;

        if (!best || closer_on_screen
            || (same_on_screen && camera_distance < best_camera_distance)) {
            best = candidate;
            best_screen_distance = screen_distance;
            best_camera_distance = camera_distance;
        }
    }

    return best;
}
