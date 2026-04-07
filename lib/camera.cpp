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
    if (!initialized_) {
        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);
        distance_ = Vector3Length(offset);
        yaw_ = atan2f(offset.x, offset.z);
        float horizontal = sqrtf(offset.x * offset.x + offset.z * offset.z);
        pitch_ = atan2f(offset.y, horizontal);
        initialized_ = true;
    }

    // only rotate when left mouse button is down
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        float rotate_speed = 0.003f;
        const float dead_zone = 2.0f;

        yaw_ -= delta.x * rotate_speed;
        if (fabsf(delta.y) > dead_zone) {
            pitch_ += delta.y * rotate_speed;
        }

        pitch_ = Clamp(pitch_, -1.5f, 1.5f);
    }

    float wheel = GetMouseWheelMove();
    const float zoom_speed = 1.8f;
    distance_ -= wheel * zoom_speed;
    distance_ = Clamp(distance_, 10.0f, 100.0f);

    camera_.position.x = camera_.target.x + distance_ * cosf(pitch_) * sinf(yaw_);
    camera_.position.y = camera_.target.y + distance_ * sinf(pitch_);
    camera_.position.z = camera_.target.z + distance_ * cosf(pitch_) * cosf(yaw_);
}
