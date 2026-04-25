#pragma once

#include <raylib.h>
#include <string>

#include "utils/game_config.hpp"

enum class button_operation {
    NONE,
    SET_TRUE,
    SET_FALSE,
    TOGGLE
};

class button {
private:
    int x;
    int y;
    int width;
    int height;
    Color color;
    button_operation operation = button_operation::NONE;
    std::string text;

    bool hovered = false;
    bool flag = false;
    config_set_funcptr callback = nullptr;

private:
    bool is_hovered(int mouse_x, int mouse_y) const {
        return mouse_x >= x &&
               mouse_x <= x + width &&
               mouse_y >= y &&
               mouse_y <= y + height;
    }

public:
    button(int x,
           int y,
           int width,
           int height,
           Color color,
           button_operation op,
           const char* text,
           config_set_funcptr callback = nullptr) :
        x(x), y(y),
        width(width), height(height),
        color(color), operation(op),
        text(text), callback(callback) {}
    void draw();
};