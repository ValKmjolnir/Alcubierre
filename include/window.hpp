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
    void begin_scene_pass();
    void end_scene_pass();

    // Bloom
    void apply_bloom();

    // Alcubierre warp lens post-processing
    void set_warp_enabled(bool enabled);
    bool is_warp_enabled() const;
    void set_velocity(const Vector3& velocity);
    Vector3 get_velocity() const;
    void set_warp_factor(float factor);
    float get_warp_factor() const;
    void set_bubble_radius(float radius);
    float get_bubble_radius() const;
    void set_wall_thickness(float thickness);
    float get_wall_thickness() const;
    void set_exposure(float exposure);
    float get_exposure() const;
    void apply_warp_to_texture(const RenderTexture2D& texture);

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
    int loc_bloom_intensity_;
    int loc_brightness_threshold_;
    int loc_texel_size_;
    int loc_blur_radius_;

    // Alcubierre warp lens shader
    Shader warp_shader_;
    bool warp_shaders_loaded_ = false;
    int loc_velocity_;
    int loc_warp_factor_;
    int loc_bubble_radius_;
    int loc_wall_thickness_;
    int loc_exposure_;

    // Warp settings
    bool warp_enabled_ = false;
    Vector3 velocity_ = { 0.0f, 0.0f, 1.0f };
    float warp_factor_ = 1.0f;
    float bubble_radius_ = 0.5f;
    float wall_thickness_ = 0.1f;
    float exposure_ = 1.0f;

    void init_bloom();
    void unload_bloom();
    void init_warp();
    void unload_warp();

    void draw_texture_on_main_screen(const RenderTexture2D& texture);
    void draw_texture_to_specific_screen(const RenderTexture2D& texture, int width, int height);
};
