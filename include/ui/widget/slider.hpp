#pragma once

#include <raylib.h>

#include "ui/widget/widget.hpp"
#include "utils/game_config.hpp"

class slider: public widget {
private:
    int x;
    int y;
    int width;
    int height;
    Color color;
    float ratio = 0.5;

    int min_value = 0;
    int max_value = 100;

    config_get_int_funcptr get;
    config_set_int_funcptr set;

public:
    bool is_hovered(int mouse_x, int mouse_y) const override {
        return mouse_x >= x &&
               mouse_x <= x + width &&
               mouse_y >= y + height / 4 &&
               mouse_y <= y + height / 4 * 3;
    }

public:
    slider(int x,
           int y,
           int width,
           int height,
           Color color,
           int min_value,
           int max_value,
           config_get_int_funcptr gf,
           config_set_int_funcptr cf) :
        x(x), y(y),
        width(width), height(height),
        color(color),
        min_value(min_value), max_value(max_value),
        get(gf), set(cf) {}
    void draw();
    void mouse_release_call_back() override;
};
