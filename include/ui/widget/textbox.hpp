#pragma once

#include <raylib.h>

#include <string>

#include "ui/widget/widget.hpp"

class textbox: public widget {
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
    void draw() override;
    void move(int dx, int dy) override;
    void set_text(const std::string& value) { text = value; }
    void set_color(Color value) { color = value; }
};