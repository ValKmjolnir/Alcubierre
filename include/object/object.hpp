#pragma once

#include <raylib.h>
#include "camera.hpp"

class object {
public:
    object() {}
    virtual ~object() = default;

    virtual void update(float dt) {}
    virtual void draw(const camera_3d& cam, int window_height) const {}
    virtual void draw_flare(const camera_3d& cam, int rt_width, int rt_height, int window_height) const {}
    virtual bool is_flare_source() const { return false; }
};
