#include "camera.hpp"
#include "raylib.h"
#include "raymath.h"

camera_3d::camera_3d() {
    camera_.position = { 5.0f, 5.0f, 5.0f };
    camera_.target = { 0.0f, 0.0f, 0.0f };
    camera_.up = { 0.0f, 1.0f, 0.0f };
    camera_.fovy = 45.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
}

camera_3d::camera_3d(const Vector3& position, const Vector3& target, const Vector3& up, float fovy) {
    camera_.position = position;
    camera_.target = target;
    camera_.up = up;
    camera_.fovy = fovy;
    camera_.projection = CAMERA_PERSPECTIVE;
}

const Camera3D& camera_3d::get_camera() const {
    return camera_;
}

Camera3D& camera_3d::get_camera() {
    return camera_;
}

Vector3 camera_3d::position() const {
    return camera_.position;
}

void camera_3d::set_position(const Vector3& pos) {
    camera_.position = pos;
}

Vector3 camera_3d::target() const {
    return camera_.target;
}

void camera_3d::set_target(const Vector3& target) {
    camera_.target = target;
}

Vector3 camera_3d::up() const {
    return camera_.up;
}

void camera_3d::set_up(const Vector3& up) {
    camera_.up = up;
}

float camera_3d::fovy() const {
    return camera_.fovy;
}

void camera_3d::set_fovy(float fovy) {
    camera_.fovy = fovy;
}

int camera_3d::projection() const {
    return camera_.projection;
}

void camera_3d::set_projection(int projection) {
    camera_.projection = projection;
}

void camera_3d::update(float dt) {
    // UpdateCamera(&camera_, CAMERA_THIRD_PERSON);
    // Vector3 movement = { 0.0f, 0.0f, 0.0f };
    // if (IsKeyDown(KEY_W)) { movement.x += 0.1f; }
    // if (IsKeyDown(KEY_S)) { movement.x -= 0.1f; }
    // if (IsKeyDown(KEY_A)) { movement.y -= 0.1f; }
    // if (IsKeyDown(KEY_D)) { movement.y += 0.1f; }

    // Vector3 rotation = { 0.0f, 0.0f, 0.0f };
    // rotation.y = GetMouseDelta().y * 0.05f;
    // rotation.z = GetMouseDelta().x * 0.05f;

    // float zoom_speed = 0.05f;
    // float zoom = GetMouseWheelMove() * zoom_speed;

    // UpdateCameraPro(&camera_, movement, rotation, zoom);

    float distance = Vector3Distance(camera_.position, camera_.target);
    distance += GetMouseWheelMove() * 0.00005f;
    distance = Clamp(distance, 10.0f, 100.0f);

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera_.target, camera_.position));
    camera_.position = Vector3Add(camera_.target, Vector3Scale(forward, -distance));

    UpdateCamera(&camera_, CAMERA_THIRD_PERSON);
}
