#pragma once

#include <raylib.h>
#include <raymath.h>

#include "light_base.hpp"
#include "object/object.hpp"
#include "camera.hpp"

class star : public point_light, public object {
private:
    int color_r_;
    int color_g_;
    int color_b_;
    int color_alpha_;
    float scale_ = 600.0f;
    float flare_thickness_ = 0.03f;

    Texture2D texture_;
    Texture2D flare_texture_;

private:
    void load_texture();
    void load_flare_texture();

public:
    star();
    star(const Vector3& position, int r = 255, int g = 200, int b = 100, int alpha = 255);
    ~star() override;

    int color_r() const { return color_r_; }
    int color_g() const { return color_g_; }
    int color_b() const { return color_b_; }
    int color_alpha() const { return color_alpha_; }

    void set_color(int r, int g, int b, int alpha = 255);
    void set_scale(float s) { scale_ = s; }
    float scale() const { return scale_; }

    void draw(const camera_3d& cam, int window_height) const override;
    Vector2 draw_flare(const camera_3d& cam, int rt_width, int rt_height, int window_height) const override;
    Vector3 flare_position() const override { return position_; }
    bool is_flare_source() const override { return true; }
};
