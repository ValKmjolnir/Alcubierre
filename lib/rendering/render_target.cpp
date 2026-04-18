#include "rendering/render_target.hpp"

render_target::render_target(int w, int h): width(w), height(h) {
    texture = LoadRenderTexture(w, h);
}

render_target::~render_target() {
    UnloadRenderTexture(texture);
}

void render_target::set(int w, int h) {
    UnloadRenderTexture(texture);
    texture = LoadRenderTexture(w, h);
    width = w;
    height = h;
}