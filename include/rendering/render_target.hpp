#pragma once

#include "raylib.h"

class render_target {
private:
    RenderTexture2D texture;
    int width;
    int height;

public:
    render_target(int w, int h);
    ~render_target();

    RenderTexture2D& get_texture() { return texture; }
    void set(int w, int h);
    int get_width() const { return width; }
    int get_height() const { return height; }
};