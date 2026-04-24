#pragma once

#include <raylib.h>

#include <string>

class button {
public:
    typedef void (*callback_t)();

private:
    int x;
    int y;
    int width;
    int height;
    Color color;
    std::string text;

    bool hovered = false;
    callback_t callback = nullptr;

private:
    bool is_hovered(int mouse_x, int mouse_y) const {
        return mouse_x >= x &&
               mouse_x <= x + width &&
               mouse_y >= y &&
               mouse_y <= y + height;
    }

public:
    button(int x, int y,
           int width, int height,
           Color color, const char* text,
           callback_t callback = nullptr) :
        x(x), y(y),
        width(width), height(height),
        color(color), text(text), callback(callback) {}
    void draw();
};