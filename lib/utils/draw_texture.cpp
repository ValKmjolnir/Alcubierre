#include "utils/draw_texture.hpp"

void draw_texture_to_specific_screen(const RenderTexture2D& texture, int width, int height) {
    const Rectangle src = {
        0.0f,
        0.0f,
        static_cast<float>(texture.texture.width),
        -static_cast<float>(texture.texture.height)
    };
    const Rectangle dst = {
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
    DrawTexturePro(texture.texture, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}