#include <raylib.h>

#include "utils/game_config.hpp"

game_config& game_config::singleton() {
    static game_config instance;
    return instance;
}

void game_config::set_should_exit(bool flag) {
    should_exit = flag;
}

void game_config::set_enable_grid_draw(bool flag) {
    enable_grid_draw = flag;
}

void game_config::set_enable_debug_hud(bool flag) {
    enable_debug_hud = flag;
}

void game_config::set_max_fps(int fps) {
    max_fps = fps;
    SetTargetFPS(max_fps);
}
