#include "rendering/render_pass.hpp"
#include "utils/draw_texture.hpp"

void render_pass::load() {
    output.load(width, height);
}

void render_pass::unload() {
    output.unload();
}

void render_pass::resize(int w, int h) {
    width = w;
    height = h;
    output.set(w, h);
    SetTextureFilter(output.get().texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(output.get().texture, TEXTURE_WRAP_CLAMP);
}

bool render_pass::ready() const {
    return enabled && output.ready();
}

texture_handle& render_pass::apply(const RenderTexture2D& texture, int width, int height) {
    BeginTextureMode(output.get());
    draw_texture_to_specific_screen(texture, width, height);
    EndTextureMode();
    return output;
}