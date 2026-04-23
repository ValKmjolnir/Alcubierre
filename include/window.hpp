#pragma once

#include <raylib.h>
#include <raymath.h>

#include "rendering/warp.hpp"
#include "rendering/fxaa.hpp"
#include "rendering/smaa.hpp"
#include "rendering/bloom.hpp"
#include "rendering/frame_graph.hpp"

#include <memory>

class game_window {
private:
    int width_;
    int height_;

    // Main scene texture
    RenderTexture2D scene_texture_;
    float ssaa_factor_ = 1.5f;

    // Lit object shader (for cubes, grid, etc.)
    Shader lit_shader_;
    Material lit_material_;
    bool lit_shader_loaded_ = false;
    int loc_object_color_;
    int loc_ambient_strength_;

    // Cached meshes for lit rendering
    Mesh cube_mesh_;
    bool cube_mesh_ready_ = false;

    frame_graph frame_graph_;

    void init_frame_graph();
    void unload_frame_graph();

    void load();
    void unload();
    void init_lit_shader();
    void unload_lit_shader();

public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    void begin_drawing() { BeginDrawing(); }
    void end_drawing() { EndDrawing(); }

    // 3D camera methods
    void begin_mode_3d(const Camera3D& camera) { BeginMode3D(camera); }
    void end_mode_3d() { EndMode3D(); }

    // Draw methods using lit shader
    void draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b);
    void draw_grid(int slices, float spacing) { DrawGrid(slices, spacing); }

    // Bloom post-processing
    void begin_scene_pass();
    void end_scene_pass();

    void apply();

    frame_graph& get_frame_graph() { return frame_graph_; }
    warp_renderer& get_warp_renderer();
    bloom& get_bloom_renderer();

    int width() const { return width_; }
    int height() const { return height_; }
};
