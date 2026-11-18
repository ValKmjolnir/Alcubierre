#pragma once

#include <raylib.h>
#include "camera.hpp"

class object {
public:
    object() {}
    virtual ~object() = default;

    virtual void update(float dt) {}
    virtual void draw(const camera_3d& cam, int window_height) const {}
    virtual Vector2 draw_flare(const camera_3d& cam, int rt_width, int rt_height, int window_height) const { return Vector2 { -1.0f, -1.0f }; }
    virtual bool is_flare_source() const { return false; }
    virtual Vector3 flare_position() const { return Vector3 { 0.0f, 0.0f, 0.0f }; }
    virtual bool blocks_line(const Vector3& from, const Vector3& to) const { return false; }
};
