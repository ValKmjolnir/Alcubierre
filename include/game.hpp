#pragma once

#include <memory>
#include <vector>

#include "window.hpp"
#include "scenes/scene.hpp"
#include "camera.hpp"
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

enum class warp_stage {
    idle,
    spooling,
    arriving
};

class game_scene;
class ship;

class game {
private:
    game_window window_;
    std::unique_ptr<game_scene> active_scene_;
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
    float lock_elapsed_ = 0.0f;
    bool target_locked_ = false;

    bool aligning_ = false;
    bool warp_after_alignment_ = false;
    float align_target_yaw_ = 0.0f;

    warp_stage warp_stage_ = warp_stage::idle;
    float warp_stage_time_ = 0.0f;
    Vector3 warp_start_position_ {};
    Vector3 warp_end_position_ {};
    bool warp_to_fleet_ = false;

    Vector2 press_position_ {};
    bool pick_armed_ = false;
    bool grid_press_armed_ = false;

public:
    game();
    ~game();
    void run();

private:
    void setup_scene();
    void enter_scene(std::unique_ptr<game_scene> next);
    void setup_ui();
    void setup_post_processing();

    void handle_input(float dt);
    void update(float dt);
    void render();
    void update_target_lock(float dt);
    void start_alignment(bool warp_after);
    void update_alignment(float dt);
    void start_warp_spool();
    void update_warp_sequence(float dt);
    void select_target_at_mouse();
    void set_target(ship* target);
    void clear_target();
};
