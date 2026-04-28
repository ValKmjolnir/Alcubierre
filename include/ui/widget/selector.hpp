#pragma once

#include <raylib.h>

#include "ui/widget/widget.hpp"

class selector: public widget {
private:
    int x;
    int y;
    int width;
    int height;

    Color color;

private:
    void draw_text_centered(Rectangle& target, const char* text, int fontsize, Color color);

public:
    selector(int x, int y, int width, int height, Color color) :
        x(x), y(y), width(width), height(height), color(color) {}
    void draw();
};
