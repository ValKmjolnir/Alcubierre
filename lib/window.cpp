#include "window.hpp"
#include "raylib.h"
#include "shader_loader.hpp"

game_window::game_window(int width, int height, const char* title):
    width_(width), height_(height) {
    InitWindow(width_, height_, title);
    SetTargetFPS(120);
    DisableCursor();  // hide cursor and lock to window

    // Initialize bloom and relativistic effects
    init_bloom();
    init_relativistic();
}

game_window::~game_window() {
    unload_bloom();
    unload_relativistic();
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
    bloom_composite_texture_ = LoadRenderTexture(width_, height_);

    // Set texture filtering
    SetTextureFilter(scene_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bright_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_mask_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_h_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_v_texture_.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bloom_composite_texture_.texture, TEXTURE_FILTER_BILINEAR);

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
        UnloadRenderTexture(bloom_composite_texture_);

        bloom_shaders_loaded_ = false;
    }
}

void game_window::init_relativistic() {
    // Load relativistic shaders
    auto relativistic_vs_res = try_load_shader("relativistic.vs", "relativistic.fs");
    
    relativistic_shader_ = relativistic_vs_res.shader;
    relativistic_shaders_loaded_ = relativistic_vs_res.success;
    
    if (!relativistic_shaders_loaded_) {
        TraceLog(LOG_WARNING, "Relativistic shaders failed to load");
        return;
    }

    // Get uniform locations
    loc_velocity_ = GetShaderLocation(relativistic_shader_, "velocity");
    loc_exposure_ = GetShaderLocation(relativistic_shader_, "exposure");
    
    // Set default exposure
    set_exposure(1.0f);
}

void game_window::unload_relativistic() {
    if (relativistic_shaders_loaded_) {
        UnloadShader(relativistic_shader_);
        relativistic_shaders_loaded_ = false;
    }
}

void game_window::set_relativistic_enabled(bool enabled) {
    relativistic_enabled_ = enabled;
}

bool game_window::is_relativistic_enabled() const {
    return relativistic_enabled_;
}

void game_window::set_velocity(const Vector3& velocity) {
    velocity_ = velocity;
}

Vector3 game_window::get_velocity() const {
    return velocity_;
}

void game_window::set_exposure(float exposure) {
    exposure_ = exposure;
}

float game_window::get_exposure() const {
    return exposure_;
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

void game_window::begin_scene_pass() {
    // Always render to texture, regardless of bloom enabled state
    // This allows toggling bloom without changing the render path
    BeginTextureMode(scene_texture_);
    ClearBackground(BLACK);
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // enable 4x MSAA
}

void game_window::end_scene_pass() {
    EndTextureMode();
}

void game_window::apply_bloom() {
    int scene_width = scene_texture_.texture.width;
    int scene_height = scene_texture_.texture.height;
    int bloom_width = bright_texture_.texture.width;
    int bloom_height = bright_texture_.texture.height;

    if (!bloom_enabled_ || !bloom_shaders_loaded_) {
        // No bloom - just apply relativistic to scene texture and draw
        // if (relativistic_enabled_ && relativistic_shaders_loaded_) {
        //     apply_relativistic();
        // } else {
            DrawTexturePro(
                scene_texture_.texture,
                { 0, 0, (float)scene_texture_.texture.width, -(float)scene_texture_.texture.height },
                { 0, 0, (float)width_, (float)height_ },
                { 0, 0 },
                0.0f,
                WHITE
            );
        // }
        return;
    }

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

    BeginTextureMode(bloom_composite_texture_);
    ClearBackground(BLACK);

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
    EndTextureMode();

    // Step 5: Apply relativistic effect if enabled, otherwise draw directly
    if (relativistic_enabled_ && relativistic_shaders_loaded_) {
        apply_relativistic_to_texture(bloom_composite_texture_);
    } else {
        // Draw the composited texture to screen
        DrawTexturePro(
            bloom_composite_texture_.texture,
            { 0, 0, (float)width_, -(float)height_ },
            { 0, 0, (float)width_, (float)height_ },
            { 0, 0 },
            0.0f,
            WHITE
        );
    }
}

void game_window::apply_relativistic() {
    apply_relativistic_to_texture(scene_texture_);
}

void game_window::apply_relativistic_to_texture(const RenderTexture2D& texture) {
    if (!relativistic_enabled_ || !relativistic_shaders_loaded_) {
        // Just draw the texture directly
        DrawTexturePro(
            texture.texture,
            { 0, 0, (float)texture.texture.width, -(float)texture.texture.height },
            { 0, 0, (float)width_, (float)height_ },
            { 0, 0 },
            0.0f,
            WHITE
        );
        return;
    }

    int tex_width = texture.texture.width;
    int tex_height = texture.texture.height;

    // Set shader uniforms
    float velocity_vec[3] = { velocity_.x, velocity_.y, velocity_.z };
    SetShaderValue(relativistic_shader_, loc_velocity_, velocity_vec, SHADER_UNIFORM_VEC3);
    SetShaderValue(relativistic_shader_, loc_exposure_, &exposure_, SHADER_UNIFORM_FLOAT);

    // Apply relativistic effect
    BeginShaderMode(relativistic_shader_);
    DrawTexturePro(
        texture.texture,
        { 0, 0, (float)tex_width, -(float)tex_height },
        { 0, 0, (float)width_, (float)height_ },
        { 0, 0 },
        0.0f,
        WHITE
    );
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
