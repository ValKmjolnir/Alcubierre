#pragma once

#include <raylib.h>
#include "rendering/render_pass.hpp"

class downsample_renderer: public render_pass {
private:
    Shader downsample_shader_;
    bool downsample_shader_loaded_ = false;
    int loc_resolution_ = -1;

public:
    downsample_renderer(const char* n, int w, int h): render_pass(n, w, h) {}
    void load() override;
    void unload() override;
    bool ready() const override {
        return enabled && downsample_shader_loaded_ && output.ready();
    }
    texture_handle& apply(const RenderTexture2D& texture, int width, int height) override;
};
