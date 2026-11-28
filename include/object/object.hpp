#pragma once

#include <raylib.h>
#include "camera.hpp"

class object {
public:
    object() {}
    virtual ~object() = default;

    virtual void update(float dt) {}
    virtual void set_pose(const Vector3& position, float yaw_deg) {
        (void)position;
        (void)yaw_deg;
    }
    virtual void translate(const Vector3& delta) { (void)delta; }
    void set_visible(bool visible) { visible_ = visible; }
    bool visible() const { return visible_; }
    virtual void draw(const camera_3d& cam, int window_height) const {}
    virtual Vector2 draw_flare(const camera_3d& cam, int rt_width, int rt_height, int window_height) const { return Vector2 { -1.0f, -1.0f }; }
    virtual bool is_flare_source() const { return false; }
    virtual Vector3 flare_position() const { return Vector3 { 0.0f, 0.0f, 0.0f }; }
    virtual bool blocks_line(const Vector3& from, const Vector3& to) const { return false; }

private:
    bool visible_ = true;
};
