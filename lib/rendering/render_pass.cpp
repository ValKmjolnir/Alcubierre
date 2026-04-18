#include "rendering/render_pass.hpp"
#include "utils/draw_texture.hpp"

void render_pass::load() {
    output.load(width, height);
}

void render_pass::unload() {
    output.unload();
}

bool render_pass::ready() const {
    return output.ready();
}

texture_handle& render_pass::apply(const RenderTexture2D& texture, int width, int height) {
    BeginTextureMode(output.get());
    draw_texture_to_specific_screen(texture, width, height);
    EndTextureMode();
    return output;
}