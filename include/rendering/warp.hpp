#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rendering/texture_handle.hpp"
#include "rendering/render_pass.hpp"

class warp_renderer: public render_pass {
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
    void update_warp_factor(float dt);

public:
    warp_renderer(const char* n, int w, int h): render_pass(n, w, h) {}
    ~warp_renderer() = default;
    void load() override;
    void unload() override;
    bool ready() const override {
        return enabled && warp_shaders_loaded_ && output.ready();
    }
    texture_handle& apply(const RenderTexture2D& texture, int width, int height) override;
};