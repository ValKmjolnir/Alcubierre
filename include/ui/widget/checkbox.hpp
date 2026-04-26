#pragma once

#include <raylib.h>

#include "utils/game_config.hpp"

class checkbox {
private:
    int x;
    int y;
    int size;
    Color color;
    config_get_funcptr get = nullptr;
    config_set_funcptr set = nullptr;

    bool hovered = false;

private:
    bool is_hovered(int mouse_x, int mouse_y) const {
        return mouse_x >= x &&
               mouse_x <= x + size &&
               mouse_y >= y &&
               mouse_y <= y + size;
    }
    void draw_content(bool checked);

public:
    checkbox(int x,
             int y,
             int size,
             Color color,
             config_get_funcptr gc,
             config_set_funcptr sc) :
        x(x), y(y), size(size), color(color), get(gc), set(sc) {}
    void draw();
};
