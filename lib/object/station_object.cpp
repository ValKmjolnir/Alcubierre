#include "object/station_object.hpp"

#include <cmath>

#include "window.hpp"

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

}

station_object::station_object(game_window& window, const Vector3& position,
                               float yaw_deg)
    : window_(&window)
    , position_(position)
    , yaw_deg_(yaw_deg)
    , ring_(GenMeshTorus(kRingRadius, kRingThickness, 48, 12))
    , core_(GenMeshCylinder(kCoreRadius, kCoreHeight, 16))
    , arm_x_(GenMeshCube(kRingRadius * 2.0f, 4.0f, 6.0f))
    , arm_z_(GenMeshCube(6.0f, 4.0f, kRingRadius * 2.0f))
    , module_(GenMeshCube(kModuleSize, kModuleSize, kModuleSize)) {}

station_object::~station_object() {
    UnloadMesh(ring_);
    UnloadMesh(core_);
    UnloadMesh(arm_x_);
    UnloadMesh(arm_z_);
    UnloadMesh(module_);
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

    window_->draw_lit_mesh(ring_, base, ring_surface);
    window_->draw_lit_mesh(
        core_, MatrixMultiply(base, MatrixTranslate(0.0f, -kCoreHeight * 0.5f, 0.0f)),
        core_surface);
    window_->draw_lit_mesh(arm_x_, base, ring_surface);
    window_->draw_lit_mesh(arm_z_, base, ring_surface);

    for (int i = 0; i < 4; i++) {
        const float angle = i * PI * 0.5f;
        const Matrix local = MatrixTranslate(
            cosf(angle) * kRingRadius, 0.0f, sinf(angle) * kRingRadius);
        window_->draw_lit_mesh(module_, MatrixMultiply(base, local), ring_surface);
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
