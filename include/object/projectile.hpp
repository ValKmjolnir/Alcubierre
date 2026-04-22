#pragma once

#include <raylib.h>
#include <raymath.h>

#include "object/object.hpp"

class projectile: public object {
public:
    projectile();
    projectile(const Vector3& position, const Vector3& velocity, int r = 255, int g = 100, int b = 0, int alpha = 255);
    ~projectile();

    // Projectile properties
    Vector3 position() const;
    void set_position(const Vector3& position);

    Vector3 velocity() const;
    void set_velocity(const Vector3& velocity);

    int color_r() const;
    int color_g() const;
    int color_b() const;
    int color_alpha() const;
    void set_color(int r, int g, int b, int alpha = 255);

    float radius() const;
    void set_radius(float radius);

    float intensity() const;
    void set_intensity(float intensity);

    // Projectile state
    bool is_active() const;
    void set_active(bool active);

    float lifetime() const;
    void set_lifetime(float lifetime);

    float age() const;
    void set_age(float age);

    // Trail effect
    void set_trail_enabled(bool enabled);
    bool is_trail_enabled() const;
    void set_trail_length(float length);
    float trail_length() const;
    void set_trail_color(int r, int g, int b, int alpha = 255);

    // Shader
    void load_shader(const char* vs_path, const char* fs_path);
    void unload_shader();
    bool is_shader_loaded() const;

    // Update and draw
    void update(float dt);
    void draw() const;

private:
    Vector3 position_;
    Vector3 velocity_;
    int color_r_;
    int color_g_;
    int color_b_;
    int color_alpha_;
    float radius_;
    float intensity_;
    bool active_;
    float lifetime_;
    float age_;
    bool trail_enabled_;
    float trail_length_;
    int trail_color_r_;
    int trail_color_g_;
    int trail_color_b_;
    int trail_color_alpha_;

    // Shader members
    Shader shader_;
    bool shader_loaded_ = false;
    int loc_mvp = -1;
    int loc_color = -1;
    int loc_intensity = -1;
    int loc_glow_radius = -1;
    int loc_position = -1;
    int loc_velocity = -1;
    int loc_age = -1;
    int loc_lifetime = -1;

    int calculate_age_alpha() const;
    void draw_trail() const;
};
