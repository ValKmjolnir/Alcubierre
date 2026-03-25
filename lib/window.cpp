#include "window.hpp"
#include "raylib.h"
#include "shader_loader.hpp"

game_window::game_window(int width, int height, const char* title):
    width_(width), height_(height) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // enable 4x MSAA
    InitWindow(width_, height_, title);
    SetTargetFPS(120);
    DisableCursor();  // hide cursor and lock to window

    // Initialize bloom
    init_bloom();
}

game_window::~game_window() {
    unload_bloom();
    CloseWindow();
}

void game_window::init_bloom() {
    // Full resolution scene texture - preserves MSAA quality
    scene_texture_ = LoadRenderTexture(width_ * 2, height_ * 2);

    // Half resolution bloom textures - performance optimization
    bright_texture_ = LoadRenderTexture(width_, height_);
    bloom_mask_texture_ = LoadRenderTexture(width_, height_);
    bloom_h_texture_ = LoadRenderTexture(width_, height_);
    bloom_v_texture_ = LoadRenderTexture(width_, height_);

    // Set texture filtering
    SetTextureFilter(scene_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bright_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_mask_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_h_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_v_texture_.texture, TEXTURE_FILTER_BILINEAR);

    // Load bloom shaders
    auto bloom_extract_res = try_load_shader("bloom.vs", "bloom_extract.fs");
    auto bloom_blur_h_res = try_load_shader("bloom.vs", "bloom_blur_h.fs");
    auto bloom_blur_v_res = try_load_shader("bloom.vs", "bloom_blur_v.fs");
    auto bloom_composite_res = try_load_shader("bloom.vs", "bloom.fs");

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

void game_window::unload_bloom() {
    if (bloom_shaders_loaded_) {
        UnloadShader(bloom_extract_shader_);
        UnloadShader(bloom_blur_h_shader_);
        UnloadShader(bloom_blur_v_shader_);
        UnloadShader(bloom_composite_shader_);

        UnloadRenderTexture(scene_texture_);
        UnloadRenderTexture(bright_texture_);
        UnloadRenderTexture(bloom_mask_texture_);
        UnloadRenderTexture(bloom_h_texture_);
        UnloadRenderTexture(bloom_v_texture_);

        bloom_shaders_loaded_ = false;
    }
}

void game_window::set_bloom_enabled(bool enabled) {
    bloom_enabled_ = enabled;
}

bool game_window::is_bloom_enabled() const {
    return bloom_enabled_;
}

void game_window::set_bloom_threshold(float threshold) {
    bloom_threshold_ = threshold;
}

void game_window::set_bloom_intensity(float intensity) {
    bloom_intensity_ = intensity;
}

void game_window::set_bloom_blur_radius(float radius) {
    bloom_blur_radius_ = radius;
}

void game_window::begin_bloom_pass() {
    // Always render to texture, regardless of bloom enabled state
    // This allows toggling bloom without changing the render path
    BeginTextureMode(scene_texture_);
    ClearBackground(BLACK);
}

void game_window::end_bloom_pass() {
    EndTextureMode();

    if (bloom_enabled_ && bloom_shaders_loaded_) {
        apply_bloom();
    } else {
        // Draw scene texture directly to screen without bloom
        // Don't use BeginShaderMode with null shader - just draw directly
        DrawTexturePro(
            scene_texture_.texture,
            { 0, 0, (float)scene_texture_.texture.width, -(float)scene_texture_.texture.height },
            { 0, 0, (float)width_, (float)height_ },
            { 0, 0 },
            0.0f,
            WHITE
        );
    }
}

void game_window::apply_bloom() {
    int scene_width = scene_texture_.texture.width;
    int scene_height = scene_texture_.texture.height;
    int bloom_width = bright_texture_.texture.width;
    int bloom_height = bright_texture_.texture.height;

    // Step 1: Extract bright areas (downsample to half resolution)
    BeginTextureMode(bright_texture_);
    ClearBackground(BLACK);

    SetShaderValue(bloom_extract_shader_, loc_brightness_threshold_, &bloom_threshold_, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(bloom_extract_shader_);
    DrawTexturePro(
        scene_texture_.texture,
        { 0, 0, (float)scene_width, -(float)scene_height },
        { 0, 0, (float)bloom_width, (float)bloom_height },
        { 0, 0 },
        0.0f,
        WHITE
    );
    EndShaderMode();
    EndTextureMode();

    // Step 2: Horizontal blur
    BeginTextureMode(bloom_h_texture_);
    ClearBackground(BLACK);

    float texel_size_h[2] = { 1.0f / (float)bloom_width, 0.0f };
    SetShaderValue(bloom_blur_h_shader_, loc_texel_size_, texel_size_h, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_h_shader_, loc_blur_radius_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(bloom_blur_h_shader_);
    DrawTexturePro(
        bright_texture_.texture,
        { 0, 0, (float)bloom_width, -(float)bloom_height },
        { 0, 0, (float)bloom_width, (float)bloom_height },
        { 0, 0 },
        0.0f,
        WHITE
    );
    EndShaderMode();
    EndTextureMode();

    // Step 3: Vertical blur
    BeginTextureMode(bloom_v_texture_);
    ClearBackground(BLACK);

    float texel_size_v[2] = { 0.0f, 1.0f / (float)bloom_height };
    SetShaderValue(bloom_blur_v_shader_, loc_texel_size_, texel_size_v, SHADER_UNIFORM_VEC2);
    SetShaderValue(bloom_blur_v_shader_, loc_blur_radius_, &bloom_blur_radius_, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(bloom_blur_v_shader_);
    DrawTexturePro(
        bloom_h_texture_.texture,
        { 0, 0, (float)bloom_width, -(float)bloom_height },
        { 0, 0, (float)bloom_width, (float)bloom_height },
        { 0, 0 },
        0.0f,
        WHITE
    );
    EndShaderMode();
    EndTextureMode();

    // Step 4: Composite - combine scene with bloom
    // First, draw scene to screen
    DrawTexturePro(
        scene_texture_.texture,
        { 0, 0, (float)scene_width, -(float)scene_height },
        { 0, 0, (float)width_, (float)height_ },
        { 0, 0 },
        0.0f,
        WHITE
    );

    // Then, draw bloom on top with additive blending
    SetShaderValue(bloom_composite_shader_, loc_bloom_intensity_, &bloom_intensity_, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(bloom_composite_shader_);
    BeginBlendMode(BLEND_ADDITIVE);

    DrawTexturePro(
        bloom_v_texture_.texture,
        { 0, 0, (float)bloom_width, -(float)bloom_height },
        { 0, 0, (float)width_, (float)height_ },
        { 0, 0 },
        0.0f,
        WHITE
    );

    EndBlendMode();
    EndShaderMode();
}

bool game_window::should_close() const {
    return WindowShouldClose();
}

void game_window::begin_drawing() {
    BeginDrawing();
}

void game_window::end_drawing() {
    EndDrawing();
}

void game_window::begin_mode_3d(const Camera3D& camera) {
    BeginMode3D(camera);
}

void game_window::end_mode_3d() {
    EndMode3D();
}

void game_window::draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b) {
    const Color color = {
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        255
    };
    DrawCube(position, width, height, length, color);
    DrawCubeWires(position, width, height, length, MAROON);
}

void game_window::draw_grid(float spacing, int slices) {
    DrawGrid(slices, spacing);
}
