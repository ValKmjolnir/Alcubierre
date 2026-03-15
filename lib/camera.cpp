#include "camera.hpp"
#include "raylib.h"

camera_3d::camera_3d() {
    camera_.position = { 5.0f, 5.0f, 5.0f };
    camera_.target = { 0.0f, 0.0f, 0.0f };
    camera_.up = { 0.0f, 1.0f, 0.0f };
    camera_.fovy = 45.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
    is_free_mode_ = true;
}

camera_3d::camera_3d(const Vector3& position, const Vector3& target, const Vector3& up, float fovy) {
    camera_.position = position;
    camera_.target = target;
    camera_.up = up;
    camera_.fovy = fovy;
    camera_.projection = CAMERA_PERSPECTIVE;
    is_free_mode_ = true;
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
    if (is_free_mode_) {
        UpdateCamera(&camera_, CAMERA_THIRD_PERSON);
    } else {
        const Vector3 movement = { GetMouseWheelMove() * 5.0f, 0.0f, 0.0f };
        const Vector3 rotation = { GetMouseDelta().x * 0.005f, GetMouseDelta().y * 0.005f, 0.0f };
        UpdateCameraPro(&camera_, movement, rotation, 0.0f);
    }
}

void camera_3d::set_mode_free() {
    is_free_mode_ = true;
}

void camera_3d::set_mode_orbital() {
    is_free_mode_ = false;
}
