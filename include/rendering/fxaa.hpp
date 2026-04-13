#pragma once

#include "raylib.h"

class fxaa_renderer {
private:
    Shader fxaa_shader_;
    bool fxaa_shader_loaded_ = false;
    int loc_resolution_ = -1;

    bool fxaa_enabled_ = false;

public:
    void set_enabled(bool enabled) { fxaa_enabled_ = enabled; }
    bool enabled() const { return fxaa_enabled_; }
    bool shader_loaded() const { return fxaa_shader_loaded_; }

public:
    void load();
    void unload();
    void apply(const RenderTexture2D& texture, int width, int height);
};
