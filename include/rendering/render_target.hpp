#pragma once

#include "raylib.h"

class render_target {
private:
    int width;
    int height;

public:
    render_target(int w, int h): width(w), height(h) {}
    ~render_target() = default;

    void set(int w, int h);
    int get_width() const { return width; }
    int get_height() const { return height; }
};