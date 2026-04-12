#pragma once

#include "raylib.h"
#include "raymath.h"

#include "light_base.hpp"
#include "object.hpp"
#include "camera.hpp"

class star : public point_light, public object {
private:
    int color_r_;
    int color_g_;
    int color_b_;
    int color_alpha_;

    Image billboard_;
    Texture2D texture_;

public:
    star();
    star(const Vector3& position, int r = 255, int g = 200, int b = 100, int alpha = 255);
    ~star();

    int color_r() const { return color_r_; }
    int color_g() const { return color_g_; }
    int color_b() const { return color_b_; }
    int color_alpha() const { return color_alpha_; }

    void set_color(int r, int g, int b, int alpha = 255);

    void draw(const camera_3d&) const;
};
