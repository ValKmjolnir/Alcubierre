#include "object/station_object.hpp"

#include <cmath>

#include "rendering/mesh_renderer.hpp"

namespace {

constexpr float kRingRadius = 160.0f;
constexpr float kRingThickness = 20.0f;
constexpr float kCoreRadius = 28.0f;
constexpr float kCoreHeight = 220.0f;
constexpr float kModuleSize = 28.0f;
constexpr float kBoundingRadius = 186.0f;
constexpr float kTiltDegrees = 35.0f;

const Color kRingColor { 158, 166, 178, 255 };
const Color kCoreColor { 126, 134, 146, 255 };

struct station_meshes {
    Mesh ring;
    Mesh core;
    Mesh arm_x;
    Mesh arm_z;
    Mesh module;
};

const station_meshes& cached_station_meshes() {
    static const station_meshes meshes {
        GenMeshTorus(kRingRadius, kRingThickness, 48, 12),
        GenMeshCylinder(kCoreRadius, kCoreHeight, 16),
        GenMeshCube(kRingRadius * 2.0f, 4.0f, 6.0f),
        GenMeshCube(6.0f, 4.0f, kRingRadius * 2.0f),
        GenMeshCube(kModuleSize, kModuleSize, kModuleSize)
    };
    return meshes;
}

}

station_object::station_object(mesh_renderer& renderer, const Vector3& position,
                               float yaw_deg)
    : renderer_(&renderer)
    , position_(position)
    , yaw_deg_(yaw_deg)
    , ring_(&cached_station_meshes().ring)
    , core_(&cached_station_meshes().core)
    , arm_x_(&cached_station_meshes().arm_x)
    , arm_z_(&cached_station_meshes().arm_z)
    , module_(&cached_station_meshes().module) {}

station_object::~station_object() = default;

void station_object::translate(const Vector3& delta) {
    position_ = Vector3Add(position_, delta);
}

void station_object::draw(const camera_3d& camera, int window_height) const {
    (void)camera;
    (void)window_height;

    const Matrix base = MatrixMultiply(
        MatrixMultiply(MatrixRotateY(yaw_deg_ * DEG2RAD),
                       MatrixRotateX(kTiltDegrees * DEG2RAD)),
        MatrixTranslate(position_.x, position_.y, position_.z));

    const lit_surface ring_surface { kRingColor, 0.28f, 10.0f, {} };
    const lit_surface core_surface { kCoreColor, 0.35f, 12.0f, {} };

    renderer_->draw_lit_mesh(*ring_, base, ring_surface);
    renderer_->draw_lit_mesh(
        *core_, MatrixMultiply(base, MatrixTranslate(0.0f, -kCoreHeight * 0.5f, 0.0f)),
        core_surface);
    renderer_->draw_lit_mesh(*arm_x_, base, ring_surface);
    renderer_->draw_lit_mesh(*arm_z_, base, ring_surface);

    for (int i = 0; i < 4; i++) {
        const float angle = i * PI * 0.5f;
        const Matrix local = MatrixTranslate(
            cosf(angle) * kRingRadius, 0.0f, sinf(angle) * kRingRadius);
        renderer_->draw_lit_mesh(*module_, MatrixMultiply(base, local), ring_surface);
    }
}

bool station_object::blocks_line(const Vector3& from, const Vector3& to) const {
    const Vector3 delta = Vector3Subtract(to, from);
    const float length = Vector3Length(delta);
    if (length < 1e-5f) {
        return false;
    }

    const Vector3 direction = Vector3Scale(delta, 1.0f / length);
    const Vector3 offset = Vector3Subtract(from, position_);
    const float b = Vector3DotProduct(offset, direction);
    const float c = Vector3DotProduct(offset, offset)
        - kBoundingRadius * kBoundingRadius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) {
        return false;
    }

    const float root = sqrtf(discriminant);
    const float t0 = -b - root;
    const float t1 = -b + root;
    return (t0 >= 0.0f && t0 <= length) || (t1 >= 0.0f && t1 <= length);
}
