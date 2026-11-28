#pragma once

#include <raylib.h>

class camera_3d;

class warp_anchor {
public:
    warp_anchor(Vector3 position, const char* name);

    void draw(const camera_3d& camera, int width, int height,
              const Vector3& player_position) const;
    bool hit_test(const camera_3d& camera, int width, int height,
                  Vector2 point) const;
    Vector3 position() const { return position_; }

private:
    Vector3 position_;
    const char* name_;
};
