#pragma once

#include "raylib.h"
#include "raymath.h"

#include "rendering/warp.hpp"

#include <memory>

class game_window {
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

    // Lit object shader (for cubes, grid, etc.)
    Shader lit_shader_;
    Material lit_material_;
    bool lit_shader_loaded_ = false;
    int loc_object_color_;
    int loc_ambient_strength_;

    // Cached meshes for lit rendering
    Mesh cube_mesh_;
    Mesh grid_mesh_;
    bool cube_mesh_ready_ = false;

    warp_renderer warp_renderer_;

    void init_bloom();
    void unload_bloom();
    void init_lit_shader();
    void unload_lit_shader();

public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    bool should_close() const { return WindowShouldClose(); }
    void begin_drawing() { BeginDrawing(); }
    void end_drawing() { EndDrawing(); }

    // 3D camera methods
    void begin_mode_3d(const Camera3D& camera) { BeginMode3D(camera); }
    void end_mode_3d() { EndMode3D(); }

    // Draw methods using lit shader
    void draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b);
    void draw_grid(int slices, float spacing) { DrawGrid(slices, spacing); }

    // Bloom post-processing
    void set_bloom_enabled(bool enabled) { bloom_enabled_ = enabled; }
    bool is_bloom_enabled() const { return bloom_enabled_; }
    void set_bloom_threshold(float threshold) { bloom_threshold_ = threshold; }
    void set_bloom_intensity(float intensity) { bloom_intensity_ = intensity; }
    void set_bloom_blur_radius(float radius) { bloom_blur_radius_ = radius; }
    void begin_scene_pass();
    void end_scene_pass();

    // Bloom
    void apply_bloom();

    warp_renderer& get_warp_renderer() { return warp_renderer_; }
    int width() const { return width_; }
    int height() const { return height_; }
};
