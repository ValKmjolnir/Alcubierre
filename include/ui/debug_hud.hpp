#pragma once

#include <raylib.h>
#include <unordered_map>
#include <string>

#include "window.hpp"

class debug_hud {
private:
    game_window& window;

private:
    void draw_right_padding(const char* text,
                            int y,
                            int size,
                            Color color);

public:
    debug_hud(game_window& window) : window(window) {}
    void draw(float beta, const Vector3& cam_forward);
    void draw_timings(const std::unordered_map<std::string, double>& timings);
};