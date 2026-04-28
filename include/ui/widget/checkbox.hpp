#pragma once

#include <raylib.h>

#include "ui/widget/widget.hpp"
#include "utils/game_config.hpp"

class checkbox: public widget {
private:
    int x;
    int y;
    int size;
    Color color;
    config_get_funcptr get = nullptr;
    config_set_funcptr set = nullptr;

public:
    bool is_hovered(int mouse_x, int mouse_y) const override {
        return mouse_x >= x &&
               mouse_x <= x + size &&
               mouse_y >= y &&
               mouse_y <= y + size;
    }

public:
    checkbox(int x,
             int y,
             int size,
             Color color,
             config_get_funcptr gc,
             config_set_funcptr sc) :
        x(x), y(y), size(size), color(color), get(gc), set(sc) {}
    void draw();
    void mouse_release_call_back() override;
};
