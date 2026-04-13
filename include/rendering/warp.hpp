#pragma once

#include "raylib.h"
#include "raymath.h"

class warp_renderer {
private:
    // shader
    Shader warp_shader_;
    bool warp_shaders_loaded_ = false;
    int loc_velocity_ = -1;
    int loc_view_direction_ = -1;
    int loc_warp_factor_ = -1;
    int loc_bubble_radius_ = -1;
    int loc_wall_thickness_ = -1;
    int loc_aspect_ratio_ = -1;
    int loc_exposure_ = -1;

    // settings
    bool warp_enabled_ = false;
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
    Vector3 view_direction_ = { 0.0f, 0.0f, -1.0f };
    float warp_factor_ = 0.0f;
    float bubble_radius_ = 0.5f;
    float wall_thickness_ = 0.1f;
    float exposure_ = 1.0f;

public:
    void set_velocity(Vector3 velocity) { velocity_ = velocity; }
    const auto& get_velocity() const { return velocity_; }
    void set_bubble_radius(float bubble_radius) { bubble_radius_ = bubble_radius; }
    void set_wall_thickness(float wall_thickness) { wall_thickness_ = wall_thickness; }
    void set_exposure(float exposure) { exposure_ = exposure; }
    void set_view_direction(Vector3 direction) { view_direction_ = direction; }
    const auto get_warp_factor() const { return warp_factor_; }
    bool enabled() const { return warp_enabled_; }
    bool shader_loaded() const { return warp_shaders_loaded_; }

public:
    void load();
    void unload();
    void update_warp_factor(float dt);
    void apply(const RenderTexture2D& texture, int width, int height);
};