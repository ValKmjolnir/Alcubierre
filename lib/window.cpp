#include "raylib.h"
#include "raymath.h"

#include "window.hpp"
#include "lighting_system.hpp"
#include "utils/material_manager.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

game_window::game_window(int width, int height, const char* title):
    width_(width), height_(height),
    warp_renderer_("warp", width, height),
    fxaa_renderer_("fxaa", width, height),
    smaa_renderer_("smaa", width, height) {
    InitWindow(width_, height_, title);
    SetTargetFPS(120);
    DisableCursor();  // hide cursor and lock to window

    // Initialize bloom and warp lens effects
    init_bloom();
    warp_renderer_.load();
    fxaa_renderer_.load();
    smaa_renderer_.load();
    init_lit_shader();
}

game_window::~game_window() {
    unload_bloom();
    warp_renderer_.unload();
    fxaa_renderer_.unload();
    smaa_renderer_.unload();
    unload_lit_shader();
    CloseWindow();
}

void game_window::init_bloom() {
    // SSAA texture
    scene_texture_ = LoadRenderTexture(width_ * 1.5, height_ * 1.5);

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

void game_window::unload_bloom() {
    if (bloom_shaders_loaded_) {
        UnloadRenderTexture(scene_texture_);
        UnloadRenderTexture(bright_texture_);
        UnloadRenderTexture(bloom_mask_texture_);
        UnloadRenderTexture(bloom_h_texture_);
        UnloadRenderTexture(bloom_v_texture_);
        UnloadRenderTexture(bloom_composite_texture_);

        bloom_shaders_loaded_ = false;
    }
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

void game_window::apply_bloom() {
    if (!bloom_enabled_ || !bloom_shaders_loaded_) {
        if (fxaa_renderer_.ready()) {
            fxaa_renderer_.apply(scene_texture_, width_, height_);
            // FXAA renders directly to screen, so we're done
            return;
        }

        if (smaa_renderer_.ready()) {
            smaa_renderer_.apply(scene_texture_, width_, height_);
            // SMAA renders directly to screen, so we're done
            return;
        }

        if (warp_renderer_.ready()) {
            auto& out = warp_renderer_.apply(scene_texture_, width_, height_);
            draw_texture_to_specific_screen(out.get(), width_, height_);
        } else {
            draw_texture_to_specific_screen(scene_texture_, width_, height_);
        }
        return;
    }

    // Step 1: Extract bright areas (downsample to half resolution)
    BeginTextureMode(bright_texture_);
    ClearBackground(BLACK);
    SetShaderValue(bloom_extract_shader_, loc_brightness_threshold_, &bloom_threshold_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_extract_shader_);
    draw_texture_to_specific_screen(
        scene_texture_,
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
    draw_texture_to_specific_screen(scene_texture_, width_, height_);
    // Then, draw bloom on top with additive blending
    SetShaderValue(bloom_composite_shader_, loc_bloom_intensity_, &bloom_intensity_, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(bloom_composite_shader_);
    BeginBlendMode(BLEND_ADDITIVE);
    draw_texture_to_specific_screen(bloom_v_texture_, width_, height_);
    EndBlendMode();
    EndShaderMode();
    EndTextureMode();

    // Step 5: Apply AA and warp lens effect
    // FXAA and SMAA render directly to screen, so they bypass warp
    if (fxaa_renderer_.ready()) {
        fxaa_renderer_.apply(bloom_composite_texture_, width_, height_);
        return;
    }

    if (smaa_renderer_.ready()) {
        smaa_renderer_.apply(bloom_composite_texture_, width_, height_);
        return;
    }

    // No AA - apply warp or draw directly
    if (warp_renderer_.ready()) {
        auto& out = warp_renderer_.apply(bloom_composite_texture_, width_, height_);
        draw_texture_to_specific_screen(out.get(), width_, height_);
    } else {
        // Draw the composited texture to screen
        draw_texture_to_specific_screen(bloom_composite_texture_, width_, height_);
    }
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
        TraceLog(LOG_WARNING, "Lit object shader failed to load, falling back to raylib default");
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
