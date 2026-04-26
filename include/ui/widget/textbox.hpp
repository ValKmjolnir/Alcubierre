#pragma once

#include <raylib.h>

#include <string>

class textbox {
private:
    int x;
    int y;
    int width;
    int height;
    int fontsize;
    Color color;

    std::string text;

public:
    textbox(int x,
            int y,
            int width,
            int height,
            int fontsize,
            Color color,
            const char* text) :
        x(x), y(y),
        width(width), height(height),
        fontsize(fontsize), color(color),
        text(text) {}
    void draw();
};