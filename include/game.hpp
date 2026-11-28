#pragma once

#include <memory>
#include <vector>

#include "window.hpp"
#include "scenes/scene.hpp"
#include "camera.hpp"
#include "combat/target_lock.hpp"
#include "combat/warp_driver.hpp"
#include "rendering/skybox.hpp"
#include "rendering/grid.hpp"
#include "ui/anchor_menu.hpp"
#include "ui/menu.hpp"
#include "ui/debug_window.hpp"
#include "ui/grid_toggle.hpp"
#include "ui/mouse_cursor.hpp"
#include "ui/ship_brackets.hpp"
#include "ui/ship_status.hpp"
#include "ui/start_screen.hpp"
#include "ui/target_marker.hpp"
#include "ui/target_status.hpp"
#include "utils/input_manager.hpp"

class game_scene;
class ship;

class game {
private:
    struct mouse_routing {
        bool camera_orbit = true;
        bool world_pick = true;
        bool grid_button = true;
        bool anchor_menu = true;
        bool menu_input = true;
    };

    game_window window_;
    std::unique_ptr<game_scene> active_scene_;
    std::unique_ptr<game_scene> pending_scene_;
    Vector3 pending_corridor_offset_ {};
    camera_3d camera_;
    skybox sky_;
    grid grid_;
    menu menu_;
    debug_window debug_window_;
    start_screen start_screen_;
    ship_status status_;
    grid_toggle grid_button_;
    target_marker marker_;
    target_status target_status_;
    ship_brackets brackets_;
    anchor_menu anchor_menu_;
    mouse_cursor cursor_;
    input_manager input_;
    ambient_light ambient_;

    bool started_ = false;

    ship* player_ship_ = nullptr;
    std::vector<ship*> hostiles_;
    ship* target_ship_ = nullptr;
    target_lock lock_;
    warp_driver warp_;

    Vector2 press_position_ {};
    bool pick_armed_ = false;
    bool grid_press_armed_ = false;

public:
    game();
    ~game();
    void run();

private:
    mouse_routing route_mouse() const;
    void setup_scene();
    void enter_scene(std::unique_ptr<game_scene> next);
    void adopt_scene(std::unique_ptr<game_scene> next);
    void prepare_pending_scene(bool to_fleet, const Vector3& destination);
    void setup_ui();
    void setup_post_processing();

    void handle_input(float dt);
    void update(float dt);
    void render();
    void update_target_lock(float dt);
    void handle_warp_jump();
    void select_target_at_mouse();
    void set_target(ship* target);
    void clear_target();
};
