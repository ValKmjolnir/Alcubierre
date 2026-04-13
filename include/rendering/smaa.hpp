#pragma once

#include "raylib.h"

class smaa_renderer {
private:
    // Shaders for 3 passes
    Shader smaa_edge_shader_;
    Shader smaa_blend_shader_;
    Shader smaa_resolve_shader_;
    bool smaa_shaders_loaded_ = false;

    // Uniform locations
    int loc_edge_resolution_ = -1;
    int loc_blend_resolution_ = -1;
    int loc_resolve_resolution_ = -1;
    int loc_resolve_texture1_ = -1;

    // Intermediate render textures (Pass 1 → Pass 2 → Pass 3)
    RenderTexture2D edge_texture_;
    RenderTexture2D blend_texture_;

    bool smaa_enabled_ = false;

public:
    void set_enabled(bool enabled) { smaa_enabled_ = enabled; }
    bool enabled() const { return smaa_enabled_; }
    bool shader_loaded() const { return smaa_shaders_loaded_; }

public:
    void load();
    void unload();
    void apply(const RenderTexture2D& texture, int width, int height);
};
