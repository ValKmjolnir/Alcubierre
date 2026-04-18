#pragma once

#include "raylib.h"
#include "rendering/render_pass.hpp"

class smaa_renderer: public render_pass {
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

public:
    smaa_renderer(const char* n, int w, int h): render_pass(n, w, h) {}
    void load() override;
    void unload() override;
    bool ready() const override {
        return smaa_enabled_ && smaa_shaders_loaded_;
    }
    texture_handle& apply(const RenderTexture2D& texture, int width, int height) override;
};
