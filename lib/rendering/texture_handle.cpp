#include "rendering/texture_handle.hpp"

void texture_handle::load(int width, int height) {
    if (initialized) {
        return;
    }

    target = LoadRenderTexture(width, height);
    initialized = true;
}

void texture_handle::set(int width, int height) {
    if (!initialized) {
        return;
    }

    UnloadRenderTexture(target);
    target = LoadRenderTexture(width, height);
}

void texture_handle::unload() {
    if (!initialized) {
        return;
    }

    UnloadRenderTexture(target);
    initialized = false;
}