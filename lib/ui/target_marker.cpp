#include <raylib.h>
#include <raymath.h>

#include <cmath>
#include <cstdio>

#include "ui/target_marker.hpp"
#include "camera.hpp"
#include "ship/ship.hpp"

namespace {

constexpr float kMinRadius = 28.0f;
constexpr float kMaxRadiusFraction = 0.42f;
constexpr float kArcThickness = 1.6f;
constexpr float kArcGapDegrees = 10.0f;
constexpr float kArcSweepDegrees = 80.0f;
constexpr float kEdgeMargin = 30.0f;

Color bracket_color(bool locked) {
    if (locked) {
        return Color { 235, 244, 255, 235 };
    }
    return Color { 140, 215, 255, 190 };
}

Color progress_color() {
    return Color { 0x00, 0xdd, 0xff, 200 };
}

float ease_out(float t) {
    const float inv = 1.0f - t;
    return 1.0f - inv * inv;
}

void draw_label(const char* text, float center_x, float top_y, int font_size, Color color) {
    const int text_width = MeasureText(text, font_size);
    const int x = int(center_x) - text_width / 2;
    const int y = int(top_y);
    DrawText(text, x + 1, y + 1, font_size, Color { 0, 0, 0, 170 });
    DrawText(text, x, y, font_size, color);
}

void format_distance(char* out, size_t size, float km) {
    if (km < 10.0f) {
        snprintf(out, size, "%.2f km", km);
    } else {
        snprintf(out, size, "%.1f km", km);
    }
}

void draw_offscreen(const camera_3d& camera, int width, int height,
                    const Vector3& target_center, float distance_km, bool locked) {
    const Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target(), camera.position()));
    const Vector3 right = Vector3Normalize(
        Vector3CrossProduct(forward, camera.up()));
    const Vector3 up = Vector3CrossProduct(right, forward);
    const Vector3 rel = Vector3Subtract(target_center, camera.position());

    Vector2 dir { Vector3DotProduct(rel, right), -Vector3DotProduct(rel, up) };
    if (Vector2Length(dir) < 1e-4f) {
        dir = Vector2 { 0.0f, 1.0f };
    }
    dir = Vector2Normalize(dir);

    const Vector2 screen_center { width * 0.5f, height * 0.5f };
    const float half_width = fmaxf(width * 0.5f - kEdgeMargin, 1.0f);
    const float half_height = fmaxf(height * 0.5f - kEdgeMargin, 1.0f);
    const float scale = fminf(
        half_width / fmaxf(fabsf(dir.x), 1e-4f),
        half_height / fmaxf(fabsf(dir.y), 1e-4f));
    const Vector2 tip {
        screen_center.x + dir.x * scale,
        screen_center.y + dir.y * scale
    };
    const Vector2 side { -dir.y, dir.x };
    const Color color = bracket_color(locked);

    const Vector2 tail = Vector2Subtract(tip, Vector2Scale(dir, 9.0f));
    DrawLineEx(Vector2Add(tail, Vector2Scale(side, 7.0f)),
               Vector2Add(tip, Vector2Scale(dir, 3.0f)), 2.0f, color);
    DrawLineEx(Vector2Subtract(tip, Vector2Scale(dir, 3.0f)),
               Vector2Subtract(tail, Vector2Scale(side, 7.0f)), 2.0f, color);

    char distance[32];
    format_distance(distance, sizeof(distance), distance_km);
    draw_label(distance,
               tip.x - dir.x * 22.0f,
               tip.y - dir.y * 22.0f - 6.0f,
               11,
               color);
}

}

void target_marker::draw(const camera_3d& camera, int width, int height,
                         const ship& target, float distance_km,
                         float progress, bool locked) const {
    if (target.destroyed()) {
        return;
    }

    const Vector3 center = target.aim_point();
    const Vector3 to_target = Vector3Subtract(center, camera.position());
    const Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target(), camera.position()));
    const float along = Vector3DotProduct(to_target, forward);

    if (along <= 0.01f) {
        draw_offscreen(camera, width, height, center, distance_km, locked);
        return;
    }

    const Vector2 screen = GetWorldToScreenEx(
        center, camera.get_camera(), width, height);
    if (screen.x < 0.0f || screen.x > float(width)
        || screen.y < 0.0f || screen.y > float(height)) {
        draw_offscreen(camera, width, height, center, distance_km, locked);
        return;
    }

    const float camera_distance = Vector3Length(to_target);
    const float px_per_unit = float(height)
        / (2.0f * camera_distance * tanf(camera.fovy() * DEG2RAD * 0.5f));
    float radius = 0.5f * target.ship_class().length * px_per_unit;
    radius = Clamp(radius, kMinRadius, kMaxRadiusFraction * fminf(width, height));
    radius *= 1.45f - 0.45f * ease_out(progress);

    const Color color = bracket_color(locked);

    for (int i = 0; i < 4; i++) {
        const float start = i * 90.0f + kArcGapDegrees * 0.5f;
        DrawRing(screen,
                 radius - kArcThickness,
                 radius + kArcThickness,
                 start,
                 start + kArcSweepDegrees,
                 24,
                 color);
    }

    for (int i = 0; i < 4; i++) {
        const float angle = i * 90.0f * DEG2RAD;
        const Vector2 inner {
            screen.x + cosf(angle) * (radius - 4.0f),
            screen.y + sinf(angle) * (radius - 4.0f)
        };
        const Vector2 outer {
            screen.x + cosf(angle) * (radius + 4.0f),
            screen.y + sinf(angle) * (radius + 4.0f)
        };
        DrawLineEx(inner, outer, 1.0f, color);
    }

    if (!locked) {
        const float ring_radius = radius + 7.0f;
        DrawRing(screen, ring_radius, ring_radius + 2.0f, 0.0f, 360.0f, 48,
                 Color { 0x00, 0xdd, 0xff, 45 });
        DrawRing(screen, ring_radius, ring_radius + 2.0f, -90.0f,
                 -90.0f + 360.0f * progress, 48, progress_color());
    }

    char label[96];
    char distance[32];
    format_distance(distance, sizeof(distance), distance_km);
    snprintf(label, sizeof(label), "%s  %s",
             target.ship_class().name, distance);
    draw_label(label, screen.x, screen.y + radius + 8.0f, 12, color);
}
