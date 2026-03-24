#pragma once

#include "raylib.h"
#include "raymath.h"

class game_window {
public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    bool should_close() const;
    void begin_drawing();
    void end_drawing();
    void clear_background(int r, int g, int b);

    // 3D camera methods
    void begin_mode_3d(const Camera3D& camera);
    void end_mode_3d();
    void draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b);
    void draw_grid(float spacing, int slices);

    // Bloom post-processing
    void set_bloom_enabled(bool enabled);
    bool is_bloom_enabled() const;
    void set_bloom_threshold(float threshold);
    void set_bloom_intensity(float intensity);
    void set_bloom_blur_radius(float radius);
    void begin_bloom_pass();
    void end_bloom_pass();

private:
    int width_;
    int height_;

    // Bloom members
    bool bloom_enabled_ = false;
    float bloom_threshold_ = 0.7f;
    float bloom_intensity_ = 5.0f;
    float bloom_blur_radius_ = 4.0f;

    // Render textures for bloom
    RenderTexture2D scene_texture_;
    RenderTexture2D bright_texture_;
    RenderTexture2D bloom_h_texture_;
    RenderTexture2D bloom_v_texture_;

    // Bloom shaders
    Shader bloom_extract_shader_;
    Shader bloom_blur_h_shader_;
    Shader bloom_blur_v_shader_;
    Shader bloom_composite_shader_;

    bool bloom_shaders_loaded_ = false;
    int loc_bloom_intensity_;
    int loc_brightness_threshold_;
    int loc_texel_size_;
    int loc_blur_radius_;

    void init_bloom();
    void unload_bloom();
    void apply_bloom();
};
