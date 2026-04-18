#pragma once

#include "raylib.h"
#include "rendering/texture_handle.hpp"
#include "rendering/render_pass.hpp"

class bloom: public render_pass {
private:
    // Bloom members
    float bloom_threshold_ = 0.7f;
    float bloom_intensity_ = 5.0f;
    float bloom_blur_radius_ = 4.0f;

    // Render textures for bloom
    RenderTexture2D bright_texture_;
    RenderTexture2D bloom_mask_texture_;
    RenderTexture2D bloom_h_texture_;
    RenderTexture2D bloom_v_texture_;
    RenderTexture2D bloom_composite_texture_;

    // Bloom shaders
    Shader bloom_extract_shader_;
    Shader bloom_blur_h_shader_;
    Shader bloom_blur_v_shader_;
    Shader bloom_composite_shader_;

    bool bloom_shaders_loaded_ = false;
    int loc_bloom_intensity_ = -1;
    int loc_brightness_threshold_ = -1;
    int loc_texel_size_ = -1;
    int loc_blur_radius_ = -1;

public:
    void set_bloom_threshold(float threshold) { bloom_threshold_ = threshold; }
    void set_bloom_intensity(float intensity) { bloom_intensity_ = intensity; }
    void set_bloom_blur_radius(float radius) { bloom_blur_radius_ = radius; }

public:
    bloom(const char* name, int w, int h): render_pass(name, w, h) {}
    void load();
    void unload();
    bool ready() const {
        return enabled && bloom_shaders_loaded_ && output.ready();
    }
    texture_handle& apply(const RenderTexture2D& texture, int width, int height);
};