#include "object/model_object.hpp"

#include <raymath.h>

#include <cmath>

#include "window.hpp"

namespace {

constexpr float kSpecStrength = 0.4f;
constexpr float kShininess = 14.0f;

// lit_object.fs pushes anything above ~0.5 albedo straight to white at this
// scene's star intensity; kit assets ship much brighter base colours, so bring
// them into the range the hand-authored hulls sit in.
constexpr float kAlbedoScale = 0.6f;

bool ray_hits_bounds(const Vector3& a, const Vector3& b, const BoundingBox& box) {
    const float origin[3] = { a.x, a.y, a.z };
    const float dir[3] = { b.x - a.x, b.y - a.y, b.z - a.z };
    const float lo[3] = { box.min.x, box.min.y, box.min.z };
    const float hi[3] = { box.max.x, box.max.y, box.max.z };

    float tmin = 0.0f;
    float tmax = 1.0f;
    for (int i = 0; i < 3; i++) {
        if (fabsf(dir[i]) < 1e-6f) {
            if (origin[i] < lo[i] || origin[i] > hi[i]) return false;
            continue;
        }
        float t1 = (lo[i] - origin[i]) / dir[i];
        float t2 = (hi[i] - origin[i]) / dir[i];
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }
    return tmax > 0.0f;
}

} // namespace

model_object::model_object(game_window& win, const char* path, float target_length,
                           const Vector3& position, float yaw_deg)
    : window_(&win) {
    model_ = LoadModel(path);
    if (model_.meshCount == 0) {
        return;
    }

    loaded_ = true;
    bounds_ = GetModelBoundingBox(model_);

    const Vector3 size = Vector3Subtract(bounds_.max, bounds_.min);
    const float longest = fmaxf(size.x, fmaxf(size.y, size.z));
    const float scale = (longest > 1e-6f) ? (target_length / longest) : 1.0f;

    // Kit assets are rarely centred on their origin, so recentre on the bounds
    // before placing: the position argument then means the hull centre.
    const Vector3 center = Vector3Scale(Vector3Add(bounds_.min, bounds_.max), 0.5f);

    world_ = MatrixScale(scale, scale, scale);
    world_ = MatrixMultiply(world_, MatrixTranslate(-center.x * scale, -center.y * scale, -center.z * scale));
    world_ = MatrixMultiply(world_, MatrixRotateY(yaw_deg * DEG2RAD));
    world_ = MatrixMultiply(world_, MatrixTranslate(position.x, position.y, position.z));
}

model_object::~model_object() {
    if (loaded_) {
        UnloadModel(model_);
    }
}

void model_object::draw(const camera_3d& cam, int window_height) const {
    (void)cam;
    (void)window_height;
    if (!loaded_) {
        return;
    }

    const Matrix transform = MatrixMultiply(model_.transform, world_);
    for (int i = 0; i < model_.meshCount; i++) {
        const int index = model_.meshMaterial[i];

        lit_surface surface;
        surface.albedo = Color { 255, 255, 255, 255 };
        surface.spec_strength = kSpecStrength;
        surface.shininess = kShininess;

        if (index >= 0 && index < model_.materialCount) {
            const Material& material = model_.materials[index];
            const Color color = material.maps[MATERIAL_MAP_DIFFUSE].color;
            surface.albedo = Color {
                static_cast<unsigned char>(color.r * kAlbedoScale),
                static_cast<unsigned char>(color.g * kAlbedoScale),
                static_cast<unsigned char>(color.b * kAlbedoScale),
                255
            };
            surface.texture = material.maps[MATERIAL_MAP_DIFFUSE].texture;
        }

        window_->draw_lit_mesh(model_.meshes[i], transform, surface);
    }
}

bool model_object::blocks_line(const Vector3& from, const Vector3& to) const {
    if (!loaded_) {
        return false;
    }

    const Matrix inv = MatrixInvert(world_);
    return ray_hits_bounds(Vector3Transform(from, inv), Vector3Transform(to, inv), bounds_);
}
