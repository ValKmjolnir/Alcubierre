#pragma once

#include <raylib.h>

class slider {
private:
    int x;
    int y;
    int width;
    int height;
    Color color;

    float value = 0.5;
    bool hovered = false;

private:
    bool is_hovered(int mouse_x, int mouse_y) const {
        return mouse_x >= x &&
               mouse_x <= x + width &&
               mouse_y >= y + height / 4 &&
               mouse_y <= y + height / 4 * 3;
    }

public:
    slider(int x, int y, int width, int height, Color color) :
        x(x), y(y), width(width), height(height), color(color) {}
    void draw();
};
