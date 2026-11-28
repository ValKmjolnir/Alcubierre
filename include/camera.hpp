#pragma once

#include <raylib.h>

class camera_3d {
private:
    Camera3D camera_;

    float distance_;
    float yaw_;
    float pitch_;
    bool initialized_ = false;

public:
    camera_3d();
    camera_3d(const Vector3& position, const Vector3& target, const Vector3& up, float fovy);

    // Get the underlying raylib Camera3D
    const Camera3D& get_camera() const { return camera_; }
    Camera3D& get_camera() { return camera_; }

    // Camera properties
    Vector3 position() const { return camera_.position; }
    void set_position(const Vector3& pos) { camera_.position = pos; }

    Vector3 target() const { return camera_.target; }
    void set_target(const Vector3& target) { camera_.target = target; }

    Vector3 up() const { return camera_.up; }
    void set_up(const Vector3& up) { camera_.up = up; }

    // Normalised view direction (target - position).
    Vector3 forward() const;

    float fovy() const { return camera_.fovy; }
    void set_fovy(float fovy) { camera_.fovy = fovy; }

    float distance() const { return distance_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }
    void set_orbit(float distance, float yaw, float pitch);

    int projection() const { return camera_.projection; }
    void set_projection(int projection) { camera_.projection = projection; }

    // Camera control
    // accept_mouse = false when a menu owns the mouse; yaw/pitch/zoom are skipped
    void update(float dt, bool accept_mouse = true);
};
