#include "scenes/game_scene.hpp"

#include "ship/ship.hpp"

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

void game_scene::translate(const Vector3& delta) {
    objects_.translate_all(delta);
}

void game_scene::set_preview_mode(bool preview) {
    if (player_) {
        player_->set_visible(!preview);
    }
    objects_.set_flare_sources_visible(!preview);
}

void game_scene::set_lights_active(bool active) {
    objects_.set_lights_active(active);
}
