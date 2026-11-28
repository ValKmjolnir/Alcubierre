#include "object/model_object.hpp"

#include <raymath.h>

#include <cmath>
#include <string>
#include <unordered_map>

#include "rendering/mesh_renderer.hpp"

namespace {

constexpr float kSpecStrength = 0.4f;
constexpr float kShininess = 14.0f;

std::unordered_map<std::string, Model> model_cache;

Model* load_cached_model(const char* path) {
    auto found = model_cache.find(path);
    if (found != model_cache.end()) {
        return &found->second;
    }

    Model model = LoadModel(path);
    if (model.meshCount == 0) {
        return nullptr;
    }

    auto inserted = model_cache.emplace(path, model);
    return &inserted.first->second;
}

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

model_object::model_object(mesh_renderer& renderer, const char* path, float target_length,
                           const Vector3& position, float yaw_deg)
    : renderer_(&renderer) {
    model_ = load_cached_model(path);
    if (!model_) {
        return;
    }

    loaded_ = true;
    bounds_ = GetModelBoundingBox(*model_);

    const Vector3 size = Vector3Subtract(bounds_.max, bounds_.min);
    const float longest = fmaxf(size.x, fmaxf(size.y, size.z));
    scale_ = (longest > 1e-6f) ? (target_length / longest) : 1.0f;

    // Kit assets are rarely centred on their origin, so recentre on the bounds
    // before placing: the position argument then means the hull centre.
    center_ = Vector3Scale(Vector3Add(bounds_.min, bounds_.max), 0.5f);
    position_ = position;
    yaw_deg_ = yaw_deg;
    update_world();
}

void model_object::update_world() {
    world_ = MatrixScale(scale_, scale_, scale_);
    world_ = MatrixMultiply(
        world_,
        MatrixTranslate(-center_.x * scale_, -center_.y * scale_, -center_.z * scale_));
    world_ = MatrixMultiply(world_, MatrixRotateY(yaw_deg_ * DEG2RAD));
    world_ = MatrixMultiply(
        world_, MatrixTranslate(position_.x, position_.y, position_.z));
}

void model_object::set_pose(const Vector3& position, float yaw_deg) {
    position_ = position;
    yaw_deg_ = yaw_deg;
    update_world();
}

void model_object::translate(const Vector3& delta) {
    set_pose(Vector3Add(position_, delta), yaw_deg_);
}

model_object::~model_object() = default;

void model_object::draw(const camera_3d& cam, int window_height) const {
    (void)cam;
    (void)window_height;
    if (!loaded_ || !model_) {
        return;
    }

    const Matrix transform = MatrixMultiply(model_->transform, world_);
    for (int i = 0; i < model_->meshCount; i++) {
        const int index = model_->meshMaterial[i];

        lit_surface surface;
        surface.albedo = Color { 255, 255, 255, 255 };
        surface.spec_strength = kSpecStrength;
        surface.shininess = kShininess;

        if (index >= 0 && index < model_->materialCount) {
            const Material& material = model_->materials[index];
            const Color color = material.maps[MATERIAL_MAP_DIFFUSE].color;
            surface.albedo = Color {
                static_cast<unsigned char>(color.r * kAlbedoScale),
                static_cast<unsigned char>(color.g * kAlbedoScale),
                static_cast<unsigned char>(color.b * kAlbedoScale),
                255
            };
            surface.texture = material.maps[MATERIAL_MAP_DIFFUSE].texture;
        }

        renderer_->draw_lit_mesh(model_->meshes[i], transform, surface);
    }
}

bool model_object::blocks_line(const Vector3& from, const Vector3& to) const {
    if (!loaded_) {
        return false;
    }

    const Matrix inv = MatrixInvert(world_);
    return ray_hits_bounds(Vector3Transform(from, inv), Vector3Transform(to, inv), bounds_);
}
