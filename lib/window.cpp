#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "window.hpp"
#include "lighting_system.hpp"
#include "utils/material_manager.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

game_window::game_window(int width, int height, const char* title):
    width_(width), height_(height),
    frame_graph_(width, height) {

    InitWindow(width_, height_, title);
    rlEnableBackfaceCulling();
    SetTargetFPS(80);

    load();
}

game_window::~game_window() {
    unload();

    rlDisableBackfaceCulling();
    CloseWindow();
}

void game_window::init_frame_graph() {
    frame_graph_.add_pass<fxaa_renderer>("fxaa");
    frame_graph_.add_pass<smaa_renderer>("smaa");
    frame_graph_.add_pass<warp_renderer>("warp");
    frame_graph_.add_pass<bloom>("bloom");

    frame_graph_.load();
}

void game_window::unload_frame_graph() {
    frame_graph_.unload();
}

void game_window::load() {
    // SSAA texture
    scene_texture_ = LoadRenderTexture(
        width_ * ssaa_factor_,
        height_ * ssaa_factor_
    );

    // Set texture filtering
    SetTextureFilter(scene_texture_.texture, TEXTURE_FILTER_BILINEAR);

    init_frame_graph();
    init_lit_shader();
}

void game_window::unload() {
    UnloadRenderTexture(scene_texture_);

    unload_frame_graph();
    unload_lit_shader();
}

void game_window::begin_scene_pass() {
    // Always render to texture, regardless of bloom enabled state
    // This allows toggling bloom without changing the render path
    BeginTextureMode(scene_texture_);
    ClearBackground(BLACK);
}

void game_window::end_scene_pass() {
    EndTextureMode();
}

void game_window::apply() {
    auto& out = frame_graph_.execute(scene_texture_, width_, height_);
    draw_texture_to_specific_screen(out.get(), width_, height_);
}

void game_window::draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b) {
    if (lit_shader_loaded_ && cube_mesh_ready_) {
        // Pass lighting data from lighting_system
        lighting_system::instance().apply_to_shader(lit_shader_);

        // Set object-specific uniforms
        float color_vec3[3] = { r / 255.0f, g / 255.0f, b / 255.0f };
        SetShaderValue(lit_shader_, loc_object_color_, color_vec3, SHADER_UNIFORM_VEC3);

        float ambient = 0.15f;
        SetShaderValue(lit_shader_, loc_ambient_strength_, &ambient, SHADER_UNIFORM_FLOAT);

        // Draw cube with transform
        Matrix transform = MatrixScale(width, height, length);
        transform = MatrixMultiply(transform, MatrixTranslate(position.x, position.y, position.z));
        DrawMesh(cube_mesh_, lit_material_, transform);
    } else {
        // Fallback to raylib default
        const Color color = {
            static_cast<unsigned char>(r),
            static_cast<unsigned char>(g),
            static_cast<unsigned char>(b),
            255
        };
        DrawCube(position, width, height, length, color);
    }
}

void game_window::init_lit_shader() {
    auto lit_vs_res = shader_manager::instance().load("lit_object.vs", "lit_object.fs");
    lit_shader_ = lit_vs_res.shader;
    lit_shader_loaded_ = lit_vs_res.success;

    if (!lit_shader_loaded_) {
        TraceLog(LOG_WARNING, "Lit object shader failed to load");
        return;
    }

    // Initialize material
    lit_material_ = LoadMaterialDefault();
    lit_material_.shader = lit_shader_;

    loc_object_color_ = GetShaderLocation(lit_shader_, "objectColor");
    loc_ambient_strength_ = GetShaderLocation(lit_shader_, "ambientStrength");

    // Create cube mesh (unit cube, will be scaled)
    cube_mesh_ = GenMeshCube(1.0f, 1.0f, 1.0f);
    cube_mesh_ready_ = true;
}

void game_window::unload_lit_shader() {
    if (lit_shader_loaded_) {
        safe_unload_material(lit_material_);
        lit_shader_loaded_ = false;
    }
    if (cube_mesh_ready_) {
        UnloadMesh(cube_mesh_);
        cube_mesh_ready_ = false;
    }
}

warp_renderer& game_window::get_warp_renderer() {
    return frame_graph_.get_pass<warp_renderer>("warp");
}

bloom& game_window::get_bloom_renderer() {
    return frame_graph_.get_pass<bloom>("bloom");
}