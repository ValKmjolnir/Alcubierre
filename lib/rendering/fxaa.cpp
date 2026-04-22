#include "rendering/fxaa.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

void fxaa_renderer::load() {
    output.load(width, height);

    auto res = shader_manager::instance().load("fxaa.vs", "fxaa.fs");
    fxaa_shader_ = res.shader;
    fxaa_shader_loaded_ = res.success;

    if (!fxaa_shader_loaded_) {
        TraceLog(LOG_WARNING, "FXAA shaders failed to load");
        return;
    }

    loc_resolution_ = GetShaderLocation(fxaa_shader_, "resolution");
}

void fxaa_renderer::unload() {
    output.unload();
    fxaa_shader_loaded_ = false;
}

texture_handle& fxaa_renderer::apply(const RenderTexture2D& texture, int width, int height) {
    if (!ready()) {
        BeginTextureMode(output.get());
        draw_texture_to_specific_screen(texture, width, height);
        EndTextureMode();
        return output;
    }

    float resolution[2] = { static_cast<float>(width), static_cast<float>(height) };
    SetShaderValue(fxaa_shader_, loc_resolution_, resolution, SHADER_UNIFORM_VEC2);

    BeginTextureMode(output.get());
    BeginShaderMode(fxaa_shader_);
    draw_texture_to_specific_screen(texture, width, height);
    EndShaderMode();
    EndTextureMode();

    return output;
}
