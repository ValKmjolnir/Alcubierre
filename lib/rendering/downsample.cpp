#include "rendering/downsample.hpp"
#include "utils/shader_manager.hpp"
#include "utils/draw_texture.hpp"

void downsample_renderer::load() {
    enabled = true;
    output.load(width, height);
    SetTextureFilter(output.get().texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(output.get().texture, TEXTURE_WRAP_CLAMP);

    auto res = shader_manager::instance().load("downsample.vs", "downsample.fs");
    downsample_shader_ = res.shader;
    downsample_shader_loaded_ = res.success;

    if (!downsample_shader_loaded_) {
        TraceLog(LOG_WARNING, "Downsample shaders failed to load");
        return;
    }

    loc_resolution_ = GetShaderLocation(downsample_shader_, "resolution");
}

void downsample_renderer::unload() {
    output.unload();
    downsample_shader_loaded_ = false;
}

texture_handle& downsample_renderer::apply(const RenderTexture2D& texture, int width, int height) {
    if (!ready()) {
        BeginTextureMode(output.get());
        draw_texture_to_specific_screen(texture, width, height);
        EndTextureMode();
        return output;
    }

    float resolution[2] = { static_cast<float>(width), static_cast<float>(height) };
    SetShaderValue(downsample_shader_, loc_resolution_, resolution, SHADER_UNIFORM_VEC2);

    BeginTextureMode(output.get());
    BeginShaderMode(downsample_shader_);
    draw_texture_to_specific_screen(texture, width, height);
    EndShaderMode();
    EndTextureMode();

    return output;
}
