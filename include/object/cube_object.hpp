#pragma once

#include <raylib.h>
#include <raymath.h>

#include "object/object.hpp"
#include "window.hpp"

class cube_object: public object {
private:
    game_window* window_;
    Vector3 position_;
    float width_;
    float height_;
    float length_;
    int r_;
    int g_;
    int b_;

public:
    cube_object(game_window& win,
                const Vector3& pos,
                float w, float h, float l,
                int r, int g, int b)
        : window_(&win)
        , position_(pos)
        , width_(w), height_(h), length_(l)
        , r_(r), g_(g), b_(b) {}

    void draw(const camera_3d& cam, int window_height) const override {
        (void)cam; (void)window_height;
        window_->draw_cube(position_, width_, height_, length_, r_, g_, b_);
    }

    bool blocks_line(const Vector3& from, const Vector3& to) const override {
        const float o[3] = { from.x, from.y, from.z };
        const float d[3] = { to.x - from.x, to.y - from.y, to.z - from.z };
        const float lo[3] = {
            position_.x - width_ * 0.5f,
            position_.y - height_ * 0.5f,
            position_.z - length_ * 0.5f
        };
        const float hi[3] = {
            position_.x + width_ * 0.5f,
            position_.y + height_ * 0.5f,
            position_.z + length_ * 0.5f
        };

        float tmin = 0.0f;
        float tmax = 1.0f;
        for (int i = 0; i < 3; i++) {
            if (fabsf(d[i]) < 1e-6f) {
                if (o[i] < lo[i] || o[i] > hi[i]) return false;
                continue;
            }
            float t1 = (lo[i] - o[i]) / d[i];
            float t2 = (hi[i] - o[i]) / d[i];
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax) return false;
        }
        return tmax > 0.0f;
    }
};
