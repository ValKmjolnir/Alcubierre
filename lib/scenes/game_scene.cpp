#include "scenes/game_scene.hpp"

game_scene::game_scene(lighting_system& lights)
    : objects_(lights) {}

void game_scene::update(float dt) {
    objects_.update_all(dt);
}

void game_scene::draw(const camera_3d& camera, int window_height) const {
    objects_.draw_all(camera, window_height);
}

Vector2 game_scene::draw_flares(const camera_3d& camera, int rt_width,
                                int rt_height, int window_height) const {
    return objects_.draw_flares(camera, rt_width, rt_height, window_height);
}
