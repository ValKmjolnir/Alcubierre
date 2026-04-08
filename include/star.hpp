#pragma once

#include "raylib.h"
#include "raymath.h"

#include "light_base.hpp"
#include "object.hpp"

class star : public point_light, public object {
public:
    star();
    star(const Vector3& position, float radius, int r = 255, int g = 200, int b = 100, int alpha = 255);
    ~star();

    // Star properties
    Vector3 position() const;
    void set_position(const Vector3& position);

    float radius() const;
    void set_radius(float radius);

    int color_r() const;
    int color_g() const;
    int color_b() const;
    int color_alpha() const;
    void set_color(int r, int g, int b, int alpha = 255);

    float intensity() const;
    void set_intensity(float intensity);

    // Shader
    void load_shader(const char* vs_path, const char* fs_path);
    void unload_shader();
    bool is_shader_loaded() const;

    void draw() const;

private:
    float radius_;
    int color_r_;
    int color_g_;
    int color_b_;
    int color_alpha_;
    bool active_;

    // Shader members
    Shader shader_;
    bool shader_loaded_;
    int loc_mvp;
    int loc_color;
    int loc_intensity;
    int loc_glow_radius;
    int loc_star_position;

    void draw_with_shader_internal() const;
};
