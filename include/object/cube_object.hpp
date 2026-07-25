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
};
