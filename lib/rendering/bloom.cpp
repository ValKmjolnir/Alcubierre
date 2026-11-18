#include "rendering/bloom.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

void bloom::load() {
    output.load(width, height);
    SetTextureFilter(output.get().texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(output.get().texture, TEXTURE_WRAP_CLAMP);

    // Half resolution bloom textures (blur is expensive, half-res saves ~4x fill rate)
    bright_texture_ = LoadRenderTexture(width / 2, height / 2);
    bloom_h_texture_ = LoadRenderTexture(width / 2, height / 2);
    bloom_v_texture_ = LoadRenderTexture(width / 2, height / 2);
    bloom_streak_texture_ = LoadRenderTexture(width / 4, height / 4);
    bloom_streak2_texture_ = LoadRenderTexture(width / 4, height / 4);
    bloom_flare_texture_ = LoadRenderTexture(width / 4, height / 4);
    bloom_composite_texture_ = LoadRenderTexture(width, height);

    // Set texture filtering
    SetTextureFilter(bright_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_h_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_v_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_streak_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_streak2_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_flare_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_composite_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(bright_texture_.texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(bloom_h_texture_.texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(bloom_v_texture_.texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(bloom_streak_texture_.texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(bloom_streak2_texture_.texture, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(bloom_flare_texture_.texture, TEXTURE_WRAP_CLAMP);

    // Load bloom shaders
    auto bloom_extract_res = shader_manager::instance().load("bloom.vs", "bloom_extract.fs");
    auto bloom_blur_h_res = shader_manager::instance().load("bloom.vs", "bloom_blur_h.fs");
    auto bloom_blur_v_res = shader_manager::instance().load("bloom.vs", "bloom_blur_v.fs");
    auto bloom_streak_res = shader_manager::instance().load("bloom.vs", "bloom_streak.fs");
    auto bloom_composite_res = shader_manager::instance().load("bloom.vs", "bloom.fs");

    bloom_extract_shader_ = bloom_extract_res.shader;
    bloom_blur_h_shader_ = bloom_blur_h_res.shader;
    bloom_blur_v_shader_ = bloom_blur_v_res.shader;
    bloom_streak_shader_ = bloom_streak_res.shader;
    bloom_composite_shader_ = bloom_composite_res.shader;

    // Check if all shaders are valid
    bloom_shaders_loaded_ = bloom_extract_res.success &&
                            bloom_blur_h_res.success &&
                            bloom_blur_v_res.success &&
                            bloom_streak_res.success &&
                            bloom_composite_res.success;
    if (!bloom_shaders_loaded_) {
        TraceLog(LOG_WARNING, "Bloom shaders failed to load, bloom disabled");
        return;
    }

    // Get uniform locations
    loc_bloom_intensity_ = GetShaderLocation(bloom_composite_shader_, "bloomIntensity");
    loc_brightness_threshold_ = GetShaderLocation(bloom_extract_shader_, "brightnessThreshold");
    loc_texel_size_h_ = GetShaderLocation(bloom_blur_h_shader_, "texelSize");
    loc_blur_radius_h_ = GetShaderLocation(bloom_blur_h_shader_, "blurRadius");
    loc_texel_size_v_ = GetShaderLocation(bloom_blur_v_shader_, "texelSize");
    loc_blur_radius_v_ = GetShaderLocation(bloom_blur_v_shader_, "blurRadius");
    loc_texel_size_streak_ = GetShaderLocation(bloom_streak_shader_, "texelSize");
    loc_step_streak_ = GetShaderLocation(bloom_streak_shader_, "stepPx");
    loc_sigma_streak_ = GetShaderLocation(bloom_streak_shader_, "sigmaTaps");
    loc_gain_streak_ = GetShaderLocation(bloom_streak_shader_, "gain");
    loc_warm_streak_ = GetShaderLocation(bloom_streak_shader_, "warmMix");
    loc_origin_streak_ = GetShaderLocation(bloom_streak_shader_, "flareOrigin");
}

void bloom::streak_pass(const RenderTexture2D& src, RenderTexture2D& dst,
                        float step_px, float sigma_taps, float gain, float warm_mix) {
    BeginTextureMode(dst);
    ClearBackground(BLACK);
    float texel_size[2] = { 1.0f / (float)width, 0.0f };
    SetShaderValue(bloom_streak_shader_, loc_texel_size_streak_, texel_size, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_streak_shader_, loc_step_streak_, &step_px, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom_streak_shader_, loc_sigma_streak_, &sigma_taps, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom_streak_shader_, loc_gain_streak_, &gain, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom_streak_shader_, loc_warm_streak_, &warm_mix, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom_streak_shader_, loc_origin_streak_, &streak_origin_, SHADER_UNIFORM_VEC2);
    BeginShaderMode(bloom_streak_shader_);
    draw_texture_to_specific_screen(src, dst.texture.width, dst.texture.height);
    EndShaderMode();
    EndTextureMode();
}

void bloom::begin_flare_pass() {
    BeginTextureMode(bloom_flare_texture_);
    ClearBackground(BLACK);
}

void bloom::end_flare_pass() {
    EndTextureMode();
}

void bloom::unload() {
    output.unload();
    UnloadRenderTexture(bright_texture_);
    UnloadRenderTexture(bloom_h_texture_);
    UnloadRenderTexture(bloom_v_texture_);
    UnloadRenderTexture(bloom_streak_texture_);
    UnloadRenderTexture(bloom_streak2_texture_);
    UnloadRenderTexture(bloom_flare_texture_);
    UnloadRenderTexture(bloom_composite_texture_);
    bloom_shaders_loaded_ = false;
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
    SetShaderValue(bloom_blur_h_shader_, loc_texel_size_h_, texel_size_h, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_h_shader_, loc_blur_radius_h_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);
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
    SetShaderValue(bloom_blur_v_shader_, loc_texel_size_v_, texel_size_v, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_v_shader_, loc_blur_radius_v_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_blur_v_shader_);
    draw_texture_to_specific_screen(
        bloom_h_texture_,
        bloom_v_texture_.texture.width,
        bloom_v_texture_.texture.height
    );
    EndShaderMode();
    EndTextureMode();


    // Step 4: Horizontal anamorphic streak from the flare buffer (quarter resolution)
    const float streak_pre_gain = 3.5f;
    streak_pass(bloom_flare_texture_, bloom_streak_texture_, 4.0f, 5.0f, streak_pre_gain, 0.0f);
    streak_pass(bloom_streak_texture_, bloom_streak2_texture_, 24.0f, 8.37f, bloom_streak_intensity_, 1.0f);

    // Step 5: Composite - combine scene with bloom
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
    // Then, draw the horizontal streak on top with additive blending
    float streak_composite_intensity = 1.0f;
    SetShaderValue(bloom_composite_shader_, loc_bloom_intensity_, &streak_composite_intensity, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_composite_shader_);
    BeginBlendMode(BLEND_ADDITIVE);
    draw_texture_to_specific_screen(bloom_streak2_texture_, width, height);
    EndBlendMode();
    EndShaderMode();
    EndTextureMode();

    BeginTextureMode(output.get());
    draw_texture_to_specific_screen(bloom_composite_texture_, width, height);
    EndTextureMode();
    return output;
}