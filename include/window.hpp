#pragma once

#include <raylib.h>
#include <raymath.h>

#include "rendering/warp.hpp"
#include "rendering/fxaa.hpp"
#include "rendering/bloom.hpp"
#include "rendering/downsample.hpp"
#include "rendering/frame_graph.hpp"

#include <memory>

// Surface description for the lit shader: albedo (colour x optional texture)
// and the per-surface Blinn-Phong response.
struct lit_surface {
    Color albedo { 255, 255, 255, 255 };
    float spec_strength = 0.0f;
    float shininess = 24.0f;
    Texture2D texture {};
};

class game_window {
private:
    int width_;
    int height_;

    // Main scene texture
    RenderTexture2D scene_texture_ {};
    float ssaa_factor_ = 1.25f;

    // Scene projection clip planes (world units, 1 unit = 10 m)
    float clip_near_ = 0.25f;
    float clip_far_ = 10000.0f;

    // Lit object shader (for cubes, grid, etc.)
    Shader lit_shader_;
    Material lit_material_;
    bool lit_shader_loaded_ = false;
    int loc_object_color_;
    int loc_spec_strength_;
    int loc_shininess_;

    // Stand-in albedo for surfaces without a texture
    Texture2D white_texture_ {};

    // Final tonemap blit
    Shader tonemap_shader_;
    bool tonemap_loaded_ = false;
    int loc_tonemap_exposure_ = -1;

    // Cached meshes for lit rendering
    Mesh cube_mesh_;
    bool cube_mesh_ready_ = false;

    frame_graph frame_graph_;

    void init_frame_graph();
    void unload_frame_graph();

    void load();
    void unload();
    void create_scene_texture();
    void init_lit_shader();
    void unload_lit_shader();

public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    void begin_drawing() { BeginDrawing(); }
    void end_drawing() { EndDrawing(); }

    // Resize the window and rebuild every render target that depends on it.
    // Returns false when the size is invalid, unchanged, or clamped away.
    bool set_resolution(int w, int h);

    // 3D camera methods
    // raylib's BeginMode3D hardcodes near 0.01 / far 1000, so the projection is
    // rebuilt here with the scene's own clip planes.
    void begin_mode_3d(const Camera3D& camera);
    void end_mode_3d() { EndMode3D(); }
    void set_clip_planes(float near_plane, float far_plane);

    // Draw methods using lit shader
    void draw_lit_mesh(const Mesh& mesh, const Matrix& transform, const lit_surface& surface);
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
