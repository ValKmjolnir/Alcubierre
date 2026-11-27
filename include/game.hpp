#pragma once

#include <vector>

#include "window.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "rendering/grid.hpp"
#include "ui/menu.hpp"
#include "ui/debug_window.hpp"
#include "ui/ship_status.hpp"
#include "ui/target_marker.hpp"
#include "ui/target_status.hpp"
#include "utils/input_manager.hpp"

class ship;

class game {
private:
    game_window window_;
    scene scene_;
    camera_3d camera_;
    skybox sky_;
    grid grid_;
    menu menu_;
    debug_window debug_window_;
    ship_status status_;
    target_marker marker_;
    target_status target_status_;
    input_manager input_;
    ambient_light ambient_;

    ship* player_ship_ = nullptr;
    std::vector<ship*> hostiles_;
    ship* target_ship_ = nullptr;
    float lock_elapsed_ = 0.0f;
    bool target_locked_ = false;

    Vector2 press_position_ {};
    bool pick_armed_ = false;

public:
    game();
    ~game();
    void run();

private:
    void setup_scene();
    void setup_ui();
    void setup_post_processing();

    void handle_input(float dt);
    void update(float dt);
    void render();
    void update_target_lock(float dt);
    void select_target_at_mouse();
    void set_target(ship* target);
    void clear_target();
};
