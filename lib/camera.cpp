#include <raylib.h>
#include <raymath.h>

#include "camera.hpp"

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

void camera_3d::set_orbit(float distance, float yaw, float pitch) {
    distance_ = distance;
    yaw_ = yaw;
    pitch_ = pitch;
    initialized_ = true;
}

void camera_3d::update(float dt, bool accept_mouse) {
    // lazy initialization
    if (!initialized_) {
        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);
        distance_ = Vector3Length(offset);
        yaw_ = atan2f(offset.x, offset.z);
        float horizontal = sqrtf(offset.x * offset.x + offset.z * offset.z);
        pitch_ = atan2f(offset.y, horizontal);
        initialized_ = true;
    }

    // only rotate when left mouse button is down
    if (accept_mouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        const float rotate_speed = 0.003f * dt * 120.0f;
        const float dead_zone = 1.0f;

        yaw_ -= delta.x * rotate_speed;
        if (fabsf(delta.y) > dead_zone) {
            pitch_ += delta.y * rotate_speed;
        }

        pitch_ = Clamp(pitch_, -1.5f, 1.5f);
    }

    float wheel = accept_mouse ? GetMouseWheelMove() : 0.0f;
    if (wheel != 0.0f) {
        // Multiplicative so the same notch feels identical at 10 units and 3 km
        distance_ *= powf(1.15f, -wheel);
        distance_ = Clamp(distance_, 8.0f, 6000.0f);
    }

    camera_.position.x = camera_.target.x + distance_ * cosf(pitch_) * sinf(yaw_);
    camera_.position.y = camera_.target.y + distance_ * sinf(pitch_);
    camera_.position.z = camera_.target.z + distance_ * cosf(pitch_) * cosf(yaw_);
}
