#include "rendering/warp.hpp"
#include "shader_loader.hpp"
#include "utils.hpp"

void warp_renderer::load() {
    auto warp_vs_res = try_load_shader("relativistic.vs", "relativistic.fs");

    warp_shader_ = warp_vs_res.shader;
    warp_shaders_loaded_ = warp_vs_res.success;

    if (!warp_shaders_loaded_) {
        TraceLog(LOG_WARNING, "Warp lens shaders failed to load");
        return;
    }

    // Get uniform locations
    loc_velocity_ = GetShaderLocation(warp_shader_, "velocity");
    loc_view_direction_ = GetShaderLocation(warp_shader_, "viewDirection");
    loc_warp_factor_ = GetShaderLocation(warp_shader_, "warpFactor");
    loc_bubble_radius_ = GetShaderLocation(warp_shader_, "bubbleRadius");
    loc_wall_thickness_ = GetShaderLocation(warp_shader_, "wallThickness");
    loc_aspect_ratio_ = GetShaderLocation(warp_shader_, "aspectRatio");
    loc_exposure_ = GetShaderLocation(warp_shader_, "exposure");
}

void warp_renderer::unload() {
    if (warp_shaders_loaded_) {
        UnloadShader(warp_shader_);
        warp_shaders_loaded_ = false;
    }
}

void warp_renderer::update_warp_factor() {
    float warp_step = 0.01f * (7.0f - warp_factor_);

    if (IsKeyDown(KEY_PAGE_UP)) {
        warp_factor_ = fminf(warp_factor_ + warp_step, 6.0f);
    }
    if (IsKeyDown(KEY_PAGE_DOWN)) {
        warp_factor_ = fmaxf(warp_factor_ - warp_step, 0.0f);
    }
    if (warp_factor_ == 0.0f) {
        warp_enabled_ = false;
    } else {
        warp_enabled_ = true;
    }
}

void warp_renderer::apply(const RenderTexture2D& texture, int width, int height) {
    if (!warp_enabled_ || !warp_shaders_loaded_) {
        // Just draw the texture directly
        draw_texture_to_specific_screen(texture, width, height);
        return;
    }

    // Set shader uniforms
    float velocity_vec[3] = { velocity_.x, velocity_.y, velocity_.z };
    float view_dir_vec[3] = { view_direction_.x, view_direction_.y, view_direction_.z };
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    SetShaderValue(warp_shader_, loc_velocity_, velocity_vec, SHADER_UNIFORM_VEC3);
    SetShaderValue(warp_shader_, loc_view_direction_, view_dir_vec, SHADER_UNIFORM_VEC3);
    SetShaderValue(warp_shader_, loc_warp_factor_, &warp_factor_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(warp_shader_, loc_bubble_radius_, &bubble_radius_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(warp_shader_, loc_wall_thickness_, &wall_thickness_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(warp_shader_, loc_aspect_ratio_, &aspect, SHADER_UNIFORM_FLOAT);
    SetShaderValue(warp_shader_, loc_exposure_, &exposure_, SHADER_UNIFORM_FLOAT);

    // Apply warp lens effect
    BeginShaderMode(warp_shader_);
    draw_texture_to_specific_screen(texture, width, height);
    EndShaderMode();
}