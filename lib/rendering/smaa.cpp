#include "rendering/smaa.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

void smaa_renderer::load() {
    auto edge_res = shader_manager::instance().load("smaa.vs", "smaa_edge.fs");
    auto blend_res = shader_manager::instance().load("smaa.vs", "smaa_blend.fs");
    auto resolve_res = shader_manager::instance().load("smaa.vs", "smaa_resolve.fs");

    smaa_edge_shader_ = edge_res.shader;
    smaa_blend_shader_ = blend_res.shader;
    smaa_resolve_shader_ = resolve_res.shader;

    smaa_shaders_loaded_ = edge_res.success && blend_res.success && resolve_res.success;

    if (!smaa_shaders_loaded_) {
        TraceLog(LOG_WARNING, "SMAA shaders failed to load");
        return;
    }

    // Cache uniform locations
    loc_edge_resolution_ = GetShaderLocation(smaa_edge_shader_, "resolution");
    loc_blend_resolution_ = GetShaderLocation(smaa_blend_shader_, "resolution");
    loc_resolve_resolution_ = GetShaderLocation(smaa_resolve_shader_, "resolution");
}

void smaa_renderer::unload() {
    if (smaa_shaders_loaded_) {
        if (edge_texture_.id != 0) {
            UnloadRenderTexture(edge_texture_);
            edge_texture_ = { 0 };
        }
        if (blend_texture_.id != 0) {
            UnloadRenderTexture(blend_texture_);
            blend_texture_ = { 0 };
        }
        smaa_shaders_loaded_ = false;
    }
}

void smaa_renderer::apply(const RenderTexture2D& texture, int width, int height) {
    if (!smaa_enabled_ || !smaa_shaders_loaded_) {
        draw_texture_to_specific_screen(texture, width, height);
        return;
    }

    // Lazily create intermediate textures on first apply
    if (edge_texture_.id == 0) {
        edge_texture_ = LoadRenderTexture(width, height);
        blend_texture_ = LoadRenderTexture(width, height);
        SetTextureFilter(edge_texture_.texture, TEXTURE_FILTER_POINT);
        SetTextureFilter(blend_texture_.texture, TEXTURE_FILTER_POINT);
    }

    float resolution[2] = { static_cast<float>(width), static_cast<float>(height) };

    // Pass 1: Edge Detection
    BeginTextureMode(edge_texture_);
    ClearBackground(BLACK);
    SetShaderValue(smaa_edge_shader_, loc_edge_resolution_, resolution, SHADER_UNIFORM_VEC2);
    BeginShaderMode(smaa_edge_shader_);
    draw_texture_to_specific_screen(texture, width, height);
    EndShaderMode();
    EndTextureMode();

    // Pass 2: Blend Weight Calculation
    BeginTextureMode(blend_texture_);
    ClearBackground(BLACK);
    SetShaderValue(smaa_blend_shader_, loc_blend_resolution_, resolution, SHADER_UNIFORM_VEC2);
    BeginShaderMode(smaa_blend_shader_);
    draw_texture_to_specific_screen(edge_texture_, width, height);
    EndShaderMode();
    EndTextureMode();

    // Pass 3: Final Resolve
    SetShaderValue(smaa_resolve_shader_, loc_resolve_resolution_, resolution, SHADER_UNIFORM_VEC2);

    BeginShaderMode(smaa_resolve_shader_);
    // Must be called AFTER BeginShaderMode since glUniform requires active program
    SetShaderValueTexture(smaa_resolve_shader_,
        GetShaderLocation(smaa_resolve_shader_, "texture1"),
        blend_texture_.texture);
    draw_texture_to_specific_screen(texture, width, height);
    EndShaderMode();
}
