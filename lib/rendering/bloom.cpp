#include "rendering/bloom.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

void bloom::load() {
    output.load(width, height);

    // Half resolution bloom textures - performance optimization
    bright_texture_ = LoadRenderTexture(width, height);
    bloom_mask_texture_ = LoadRenderTexture(width, height);
    bloom_h_texture_ = LoadRenderTexture(width, height);
    bloom_v_texture_ = LoadRenderTexture(width, height);
    bloom_composite_texture_ = LoadRenderTexture(width, height);

    // Set texture filtering
    SetTextureFilter(bright_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_mask_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_h_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_v_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_composite_texture_.texture, TEXTURE_FILTER_BILINEAR);

    // Load bloom shaders
    auto bloom_extract_res = shader_manager::instance().load("bloom.vs", "bloom_extract.fs");
    auto bloom_blur_h_res = shader_manager::instance().load("bloom.vs", "bloom_blur_h.fs");
    auto bloom_blur_v_res = shader_manager::instance().load("bloom.vs", "bloom_blur_v.fs");
    auto bloom_composite_res = shader_manager::instance().load("bloom.vs", "bloom.fs");

    bloom_extract_shader_ = bloom_extract_res.shader;
    bloom_blur_h_shader_ = bloom_blur_h_res.shader;
    bloom_blur_v_shader_ = bloom_blur_v_res.shader;
    bloom_composite_shader_ = bloom_composite_res.shader;

    // Check if all shaders are valid
    bloom_shaders_loaded_ = bloom_extract_res.success &&
                            bloom_blur_h_res.success &&
                            bloom_blur_v_res.success &&
                            bloom_composite_res.success;
    if (!bloom_shaders_loaded_) {
        TraceLog(LOG_WARNING, "Bloom shaders failed to load, bloom disabled");
        return;
    }

    // Get uniform locations
    loc_bloom_intensity_ = GetShaderLocation(bloom_composite_shader_, "bloomIntensity");
    loc_brightness_threshold_ = GetShaderLocation(bloom_extract_shader_, "brightnessThreshold");
    loc_texel_size_ = GetShaderLocation(bloom_blur_h_shader_, "texelSize");
    loc_blur_radius_ = GetShaderLocation(bloom_blur_h_shader_, "blurRadius");
}

void bloom::unload() {
    output.unload();
    if (bloom_shaders_loaded_) {
        UnloadRenderTexture(bright_texture_);
        UnloadRenderTexture(bloom_mask_texture_);
        UnloadRenderTexture(bloom_h_texture_);
        UnloadRenderTexture(bloom_v_texture_);
        UnloadRenderTexture(bloom_composite_texture_);

        bloom_shaders_loaded_ = false;
    }
}

texture_handle& bloom::apply(const RenderTexture2D& texture, int width, int height) {
    if (!ready()) {
        BeginTextureMode(output.get());
        draw_texture_to_specific_screen(texture, width, height);
        EndTextureMode();
        return output;
    }

    // Step 1: Extract bright areas (downsample to half resolution)
    BeginTextureMode(bright_texture_);
    ClearBackground(BLACK);
    SetShaderValue(bloom_extract_shader_, loc_brightness_threshold_, &bloom_threshold_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_extract_shader_);
    draw_texture_to_specific_screen(
        texture,
        bright_texture_.texture.width,
        bright_texture_.texture.height
    );
    EndShaderMode();
    EndTextureMode();

    // Step 2: Horizontal blur
    BeginTextureMode(bloom_h_texture_);
    ClearBackground(BLACK);
    float texel_size_h[2] = { 1.0f / (float)bright_texture_.texture.width, 0.0f };
    SetShaderValue(bloom_blur_h_shader_, loc_texel_size_, texel_size_h, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_h_shader_, loc_blur_radius_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_blur_h_shader_);
    draw_texture_to_specific_screen(
        bright_texture_,
        bloom_h_texture_.texture.width,
        bloom_h_texture_.texture.height
    );
    EndShaderMode();
    EndTextureMode();

    // Step 3: Vertical blur
    BeginTextureMode(bloom_v_texture_);
    ClearBackground(BLACK);
    float texel_size_v[2] = { 0.0f, 1.0f / (float)bright_texture_.texture.height };
    SetShaderValue(bloom_blur_v_shader_, loc_texel_size_, texel_size_v, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_v_shader_, loc_blur_radius_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_blur_v_shader_);
    draw_texture_to_specific_screen(
        bloom_h_texture_,
        bloom_v_texture_.texture.width,
        bloom_v_texture_.texture.height
    );
    EndShaderMode();
    EndTextureMode();

    // Step 4: Composite - combine scene with bloom
    BeginTextureMode(bloom_composite_texture_);
    ClearBackground(BLACK);
    // First, draw scene to screen
    draw_texture_to_specific_screen(texture, width, height);
    // Then, draw bloom on top with additive blending
    SetShaderValue(bloom_composite_shader_, loc_bloom_intensity_, &bloom_intensity_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_composite_shader_);
    BeginBlendMode(BLEND_ADDITIVE);
    draw_texture_to_specific_screen(bloom_v_texture_, width, height);
    EndBlendMode();
    EndShaderMode();
    EndTextureMode();

    BeginTextureMode(output.get());
    draw_texture_to_specific_screen(bloom_composite_texture_, width, height);
    EndTextureMode();
    return output;
}