#pragma once

#include <raylib.h>
#include "rendering/render_pass.hpp"

class fxaa_renderer: public render_pass {
private:
    Shader fxaa_shader_;
    bool fxaa_shader_loaded_ = false;
    int loc_resolution_ = -1;

public:
    fxaa_renderer(const char* n, int w, int h): render_pass(n, w, h) {}
    void load() override;
    void unload() override;
    bool ready() const override {
        return enabled && fxaa_shader_loaded_ && output.ready();
    }
    texture_handle& apply(const RenderTexture2D& texture, int width, int height) override;
};
