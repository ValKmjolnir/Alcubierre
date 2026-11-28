#pragma once

#include <raylib.h>

class warp_tunnel {
public:
    warp_tunnel() = default;
    ~warp_tunnel();

    void load();
    void unload();

    void draw(const Vector3& origin, const Vector3& forward,
              float amount, float time) const;

private:
    Mesh mesh_ {};
    Material material_ {};
    MaterialMap maps_[8] {};
    Shader shader_ {};
    bool loaded_ = false;
    int loc_amount_ = -1;
    int loc_time_ = -1;
};
