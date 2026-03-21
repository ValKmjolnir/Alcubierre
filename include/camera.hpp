#pragma once

#include "raylib.h"

class camera_3d {
public:
    camera_3d();
    camera_3d(const Vector3& position, const Vector3& target, const Vector3& up, float fovy);
    
    // Get the underlying raylib Camera3D
    const Camera3D& get_camera() const;
    Camera3D& get_camera();
    
    // Camera properties
    Vector3 position() const;
    void set_position(const Vector3& pos);
    
    Vector3 target() const;
    void set_target(const Vector3& target);
    
    Vector3 up() const;
    void set_up(const Vector3& up);
    
    float fovy() const;
    void set_fovy(float fovy);
    
    int projection() const;
    void set_projection(int projection);
    
    // Camera control
    void update(float dt);

private:
    Camera3D camera_;
};
