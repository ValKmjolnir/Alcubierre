#include <raylib.h>
#include <raymath.h>

#include "game.hpp"
#include "utils/game_config.hpp"
#include "utils/units.hpp"
#include "scenes/fleet_scene.hpp"
#include "scenes/station_scene.hpp"
#include "ship/ship.hpp"

namespace {

constexpr bool kShowExitCorridor = true;

}

// ============================================================================
// Constructor
// ============================================================================

game::game()
    : window_(1600, 800, "Alcubierre Warp Drive")
    , camera_(
        Vector3 { 8.0f, 10.0f, 38.0f },
        Vector3 { 0.0f, 0.0f, 0.0f },
        Vector3 { 0.0f, 1.0f, 0.0f },
        45.0f)
    , sky_()
    , grid_(10.0f, 320.0f, Color { 120, 200, 255, 255 })
    , menu_(window_)
    , debug_window_(window_)
    , start_screen_(window_)
    , status_(window_)
    , target_status_(window_)
    , cursor_()
    , input_()
{
    game_config::singleton().set_max_fps(80);

    Image icon = LoadImage("assets/logo/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    tunnel_.load();

    setup_scene();
    setup_ui();
    setup_post_processing();
}

game::~game() {
    tunnel_.unload();
    lighting_system::instance().remove(&ambient_);
}

// ============================================================================
// Scene setup
// ============================================================================

void game::setup_scene() {
    window_.set_clip_planes(0.25f, 30000.0f);

    ambient_.set_sky_color({ 0.40f, 0.44f, 0.52f });
    ambient_.set_ground_color({ 0.10f, 0.11f, 0.13f });
    ambient_.set_intensity(1.0f);
    lighting_system::instance().add(&ambient_);

    enter_scene(std::make_unique<fleet_scene>(lighting_system::instance()));
}

void game::enter_scene(std::unique_ptr<game_scene> next) {
    next->build(window_);
    adopt_scene(std::move(next));
}

void game::adopt_scene(std::unique_ptr<game_scene> next) {
    warp_.reset();

    active_scene_ = std::move(next);

    player_ship_ = active_scene_->player();
    hostiles_ = active_scene_->hostiles();
    clear_target();
    anchor_menu_.close();

    window_.get_warp_renderer().set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    window_.get_warp_renderer().set_warp_factor(0.0f);

    if (player_ship_) {
        camera_ = camera_3d(
            Vector3 { 8.0f, 10.0f, 38.0f },
            player_ship_->position(),
            Vector3 { 0.0f, 1.0f, 0.0f },
            45.0f);
        camera_.update(0.0f, false);
    }
}

void game::prepare_pending_scene(bool to_fleet, const Vector3& corridor_end) {
    std::unique_ptr<game_scene> next = to_fleet
        ? std::unique_ptr<game_scene>(
            std::make_unique<fleet_scene>(lighting_system::instance()))
        : std::unique_ptr<game_scene>(
            std::make_unique<station_scene>(lighting_system::instance()));
    next->build(window_);

    const Vector3 player_position = next->player()
        ? next->player()->position()
        : Vector3 { 0.0f, 0.0f, 0.0f };

    if (kShowExitCorridor) {
        pending_corridor_offset_ = Vector3Subtract(corridor_end, player_position);
        next->translate(pending_corridor_offset_);
    } else {
        pending_corridor_offset_ = {};
    }

    next->set_preview_mode(true);
    next->set_lights_active(false);
    pending_scene_ = std::move(next);
}

// ============================================================================
// UI setup
// ============================================================================

void game::setup_ui() {
    static const resolution_option resolutions[] = {
        { 800, 400, "800x400" },
        { 1280, 720, "1280x720" },
        { 1600, 800, "1600x800" },
        { 1920, 1080, "1920x1080" },
    };

    menu_.add_new_textbox(40, 10, 100, 20, 18, WHITE, "Press ESC to toggle menu");

    menu_.add_new_textbox(40, 35, 100, 20, 18, GRAY, "Debug hud");
    menu_.add_new_checkbox(180, 35, 20,
        &game_config::get_enable_debug_hud,
        &game_config::set_enable_debug_hud);

    menu_.add_new_textbox(40, 60, 100, 20, 18, GRAY, "Max FPS");
    menu_.add_new_slider(180, 60, 200, 20, 10, 240,
        &game_config::get_max_fps,
        &game_config::set_max_fps);

    menu_.add_new_textbox(40, 85, 100, 20, 18, GRAY, "Resolution");
    menu_.add_new_selector(180, 85, 200, 20,
        resolutions,
        int(sizeof(resolutions) / sizeof(resolutions[0])));

    menu_.add_back_button(40, 110, 240, 30, "back");

    menu_.add_new_button(
        40, 150, 240, 30,
        button_operation::SET_TRUE,
        "exit game",
        &game_config::set_should_exit);
}

void game::setup_post_processing() {
    window_.get_bloom_renderer().set_bloom_threshold(0.85f);
    window_.get_bloom_renderer().set_bloom_intensity(1.2f);
    window_.get_bloom_renderer().set_bloom_blur_radius(15.0f);
    window_.get_bloom_renderer().set_bloom_streak_intensity(10.0f);

    window_.get_warp_renderer().set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    window_.get_warp_renderer().set_bubble_radius(0.37f);
    window_.get_warp_renderer().set_wall_thickness(0.04f);
    window_.get_warp_renderer().set_exposure(2.0f);
}

// ============================================================================
// Per-frame methods
// ============================================================================

game::mouse_routing game::route_mouse() const {
    const bool menu_open = menu_.get_show_menu();
    const bool debug_owns = debug_window_.captures_mouse();
    const bool anchor_open = anchor_menu_.visible();
    const bool warp_active = warp_.active();

    mouse_routing routing;
    routing.menu_input = !debug_owns;
    routing.camera_orbit = !menu_open && !debug_owns && !anchor_open;
    routing.world_pick = !menu_open && !debug_owns && !anchor_open && !warp_active;
    routing.grid_button = routing.world_pick;
    routing.anchor_menu = !menu_open && !debug_owns && !anchor_open && !warp_active;
    return routing;
}

void game::handle_input(float dt) {
    auto& config = game_config::singleton();

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (anchor_menu_.visible()) {
            anchor_menu_.close();
        } else {
            menu_.set_show_menu(!menu_.get_show_menu());
        }
    }
    if (IsKeyPressed(KEY_F1)) {
        config.set_enable_debug_hud(!config.get_enable_debug_hud());
        anchor_menu_.close();
    }
    if (IsKeyPressed(KEY_B)) {
        config.set_enable_bloom(!config.get_enable_bloom());
    }
    if (IsKeyPressed(KEY_F)) {
        config.set_enable_fxaa(!config.get_enable_fxaa());
    }

    debug_window_.set_visible(config.get_enable_debug_hud());

    input_.hide_mouse();

    if (!started_) {
        if (!menu_.get_show_menu()) {
            switch (start_screen_.update()) {
            case start_action::start:
                started_ = true;
                break;
            case start_action::settings:
                menu_.set_show_menu(true);
                break;
            case start_action::quit:
                config.set_should_exit(true);
                break;
            case start_action::none:
                break;
            }
        } else {
            start_screen_.cancel_input();
        }
    } else {
        const mouse_routing routing = route_mouse();

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            const Vector2 mouse = GetMousePosition();
            const warp_anchor* anchor = active_scene_->anchor();
            if (anchor_menu_.visible()) {
                anchor_menu_.close();
            } else if (routing.anchor_menu
                       && anchor
                       && anchor->hit_test(camera_, window_.width(),
                                           window_.height(), mouse)) {
                anchor_menu_.open(mouse, window_.width(), window_.height());
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            press_position_ = GetMousePosition();
            const int mouse_x = int(press_position_.x);
            const int mouse_y = int(press_position_.y);
            const bool over_grid = grid_button_.contains(status_.bounds(), mouse_x, mouse_y);
            grid_press_armed_ = over_grid && routing.grid_button;
            pick_armed_ = routing.world_pick
                && !over_grid
                && !status_.contains(mouse_x, mouse_y)
                && !target_status_.contains(mouse_x, mouse_y);
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            const Vector2 release_position = GetMousePosition();
            if (grid_press_armed_
                && grid_button_.contains(status_.bounds(),
                                         int(release_position.x),
                                         int(release_position.y))) {
                config.set_enable_grid_draw(!config.get_enable_grid_draw());
            }
            if (pick_armed_ && Vector2Distance(press_position_, release_position) < 4.0f) {
                select_target_at_mouse();
            }
            grid_press_armed_ = false;
            pick_armed_ = false;
        }

        switch (anchor_menu_.update()) {
        case anchor_action::align_to:
            if (const warp_anchor* anchor = active_scene_->anchor();
                anchor && player_ship_) {
                if (warp_.start_alignment(
                        *player_ship_, anchor->position(), false, false)) {
                    pending_scene_.reset();
                    pending_corridor_offset_ = {};
                }
            }
            break;
        case anchor_action::warp_to:
            if (const warp_anchor* anchor = active_scene_->anchor();
                anchor && player_ship_) {
                const bool to_fleet = active_scene_->warp_to_fleet();
                if (warp_.start_alignment(
                        *player_ship_, anchor->position(), true, to_fleet)) {
                    prepare_pending_scene(
                        to_fleet,
                        warp_driver::corridor_end_position(
                            player_ship_->position(), anchor->position()));
                }
            }
            break;
        case anchor_action::none:
            break;
        }
    }

    if (started_) {
        auto& warp = window_.get_warp_renderer();

        if (!warp_.active()) {
            warp.update_warp_factor(dt);

            // Velocity: beta (UP/DOWN), direction (LEFT/RIGHT)
            Vector3 vel = warp.get_velocity();
            float beta = Vector3Length(vel);
            float dirX = 0.0f, dirZ = 1.0f;
            if (beta > 0.001f) {
                dirX = vel.x / beta;
                dirZ = vel.z / beta;
            }
            float beta_step = 0.0025f * dt * 120.0f;
            if (IsKeyDown(KEY_UP)) {
                beta = fminf(beta + beta_step, 10.0f);
            }
            if (IsKeyDown(KEY_DOWN)) {
                beta = fmaxf(beta - beta_step, 0.0f);
            }
            if (IsKeyDown(KEY_RIGHT) && beta > 0.001f) {
                float angle = atan2f(dirX, dirZ) + 0.0025f * dt * 120.0f;
                dirX = sinf(angle);
                dirZ = cosf(angle);
            }
            if (IsKeyDown(KEY_LEFT) && beta > 0.001f) {
                float angle = atan2f(dirX, dirZ) - 0.0025f * dt * 120.0f;
                dirX = sinf(angle);
                dirZ = cosf(angle);
            }
            vel.x = dirX * beta;
            vel.z = dirZ * beta;
            warp.set_velocity(vel);
        }
    }
}

void game::update(float dt) {
    auto& config = game_config::singleton();
    auto& frame_graph = window_.get_frame_graph();
    frame_graph.set_enable("bloom", config.get_enable_bloom());
    frame_graph.set_enable("fxaa", config.get_enable_fxaa());

    if (!started_) {
        if (player_ship_) {
            camera_.set_target(player_ship_->position());
        }
        camera_.update(dt, false);
        lighting_system::instance().set_view_position(camera_.position());

        const Vector3 cam_forward = camera_.forward();
        auto& warp = window_.get_warp_renderer();
        warp.set_view_direction(cam_forward);
        warp.set_fov_y(camera_.fovy());
        return;
    }

    if (player_ship_) {
        switch (warp_.update(dt, *player_ship_,
                             window_.get_warp_renderer())) {
        case warp_driver::event::started:
            active_scene_->on_warp_start();
            player_ship_->clear_targets();
            camera_.set_target(player_ship_->position());
            anchor_menu_.close();
            break;
        case warp_driver::event::jump:
            handle_warp_jump();
            break;
        case warp_driver::event::finished:
        case warp_driver::event::none:
            break;
        }
    }

    if (player_ship_) {
        camera_.set_target(player_ship_->position());
        status_.update(dt, player_ship_->health(), player_ship_->max_health());
    }

    update_target_lock(dt);

    float distance_km = 0.0f;
    if (target_ship_ && player_ship_) {
        distance_km = units_to_km(Vector3Distance(
            player_ship_->position(), target_ship_->aim_point()));
    }
    target_status_.update(dt, target_ship_, distance_km,
                          lock_.progress(), lock_.locked());

    camera_.update(dt, route_mouse().camera_orbit);
    lighting_system::instance().set_view_position(camera_.position());
    active_scene_->update(dt);

    auto& warp = window_.get_warp_renderer();
    warp.set_view_direction(camera_.forward());
    warp.set_fov_y(camera_.fovy());
}

void game::update_target_lock(float dt) {
    if (!target_ship_) {
        return;
    }
    if (target_ship_->destroyed()) {
        clear_target();
        return;
    }
    lock_.update(dt);
}

void game::handle_warp_jump() {
    const float orbit_distance = camera_.distance();
    const float orbit_yaw = camera_.yaw();
    const float orbit_pitch = camera_.pitch();
    const bool to_fleet = warp_.to_fleet();
    const float arrival_yaw = player_ship_->yaw_deg();

    pending_scene_->set_preview_mode(false);
    if (kShowExitCorridor) {
        pending_scene_->translate(Vector3Negate(pending_corridor_offset_));
    }
    adopt_scene(std::move(pending_scene_));
    active_scene_->set_lights_active(true);
    pending_corridor_offset_ = {};

    if (player_ship_) {
        if (to_fleet) {
            player_ship_->set_yaw_deg(arrival_yaw);
        }
        camera_.set_target(player_ship_->position());
        camera_.set_orbit(orbit_distance, orbit_yaw, orbit_pitch);
        camera_.update(0.0f, false);
        warp_.begin_arrival(*player_ship_, window_.get_warp_renderer());
    }
    TraceLog(LOG_INFO, "warp: jump");
}

void game::set_target(ship* target) {
    if (target == target_ship_) {
        return;
    }
    target_ship_ = target;
    lock_.reset();
}

void game::clear_target() {
    target_ship_ = nullptr;
    lock_.reset();
}

void game::select_target_at_mouse() {
    const Vector2 mouse = GetMousePosition();
    const Ray ray = GetScreenToWorldRayEx(
        mouse, camera_.get_camera(), window_.width(), window_.height());
    const Vector3 end = Vector3Add(
        ray.position, Vector3Scale(ray.direction, 20000.0f));

    ship* best = nullptr;
    float best_distance = 0.0f;
    for (ship* candidate : hostiles_) {
        if (!candidate || candidate->destroyed()) {
            continue;
        }
        if (!candidate->blocks_line(ray.position, end)) {
            continue;
        }
        const float distance = Vector3Distance(ray.position, candidate->position());
        if (!best || distance < best_distance) {
            best = candidate;
            best_distance = distance;
        }
    }

    if (best) {
        set_target(best);
        return;
    }

    best = brackets_.pick(camera_, window_.width(), window_.height(),
                          hostiles_, target_ship_, mouse);
    if (best) {
        set_target(best);
        return;
    }

    if (target_ship_) {
        if (marker_.contains(camera_, window_.width(), window_.height(),
                             *target_ship_, lock_.progress(), mouse)) {
            return;
        }
    }

    set_target(nullptr);
}

void game::render() {
    using clock = std::chrono::high_resolution_clock;

    bool show_hud = game_config::singleton().get_enable_debug_hud();
    window_.get_frame_graph().set_timing_enabled(show_hud);

    if (!started_) {
        window_.begin_scene_pass();
        window_.begin_mode_3d(camera_.get_camera());
        sky_.draw(camera_.get_camera(), window_.get_warp_renderer().get_velocity());
        window_.end_mode_3d();
        window_.end_scene_pass();

        auto& bloom_pass = window_.get_bloom_renderer();
        bloom_pass.begin_flare_pass();
        bloom_pass.end_flare_pass();
        bloom_pass.set_streak_origin(Vector2 { -1.0f, -1.0f });

        window_.apply();
        start_screen_.draw();
        menu_.draw(true);
        cursor_.draw();
        return;
    }

    const bool warp_active = warp_.active();

    clock::time_point scene_t0;
    if (show_hud) scene_t0 = clock::now();

    window_.begin_scene_pass();
    window_.begin_mode_3d(camera_.get_camera());

    sky_.draw(camera_.get_camera(), window_.get_warp_renderer().get_velocity());
    active_scene_->draw(camera_, window_.height());
    if (kShowExitCorridor && pending_scene_) {
        pending_scene_->draw(camera_, window_.height());
    }

    if (player_ship_) {
        const Vector3 forward = Vector3Transform(
            Vector3 { 0.0f, 0.0f, -1.0f },
            MatrixRotateY(player_ship_->yaw_deg() * DEG2RAD));
        tunnel_.draw(player_ship_->position(), forward,
                     warp_.tunnel_amount(), warp_.tunnel_time());
    }

    if (!warp_active
        && game_config::singleton().get_enable_grid_draw()
        && player_ship_) {
        grid_.draw(player_ship_->position());
    }

    window_.end_mode_3d();
    window_.end_scene_pass();

    auto& bloom_pass = window_.get_bloom_renderer();
    bloom_pass.begin_flare_pass();
    Vector2 flare_origin = active_scene_->draw_flares(
        camera_, bloom_pass.flare_width(), bloom_pass.flare_height(), window_.height());
    bloom_pass.end_flare_pass();
    bloom_pass.set_streak_origin(flare_origin);


    double scene_ms = 0.0;
    if (show_hud) {
        scene_ms = std::chrono::duration<double, std::milli>(
            clock::now() - scene_t0).count();
    }

    window_.apply();

    if (!warp_active) {
        brackets_.draw(camera_, window_.width(), window_.height(),
                       hostiles_, target_ship_);

        if (const warp_anchor* anchor = active_scene_->anchor();
            anchor && player_ship_) {
            anchor->draw(camera_, window_.width(), window_.height(),
                         player_ship_->position());
        }

        status_.draw();
        grid_button_.draw(status_.bounds());

        if (target_ship_ && player_ship_) {
            const float distance_km = units_to_km(Vector3Distance(
                player_ship_->position(), target_ship_->aim_point()));
            marker_.draw(camera_, window_.width(), window_.height(),
                         *target_ship_, distance_km,
                         lock_.progress(), lock_.locked());
            target_status_.draw();
        }
    }

    menu_.draw(route_mouse().menu_input);

    if (show_hud) {
        Vector3 vel = window_.get_warp_renderer().get_velocity();
        float beta = Vector3Length(vel);

        auto timings = window_.get_frame_graph().get_pass_timings();
        timings["scene"] = scene_ms;
        debug_window_.update(beta, camera_.forward(), timings);
        debug_window_.draw();
    }

    anchor_menu_.draw();
    cursor_.draw();
}

// ============================================================================
// Main loop
// ============================================================================

void game::run() {
    while (!game_config::singleton().get_should_exit() && !WindowShouldClose()) {
        window_.begin_drawing();

        float dt = GetFrameTime();
        handle_input(dt);
        update(dt);
        render();

        window_.end_drawing();
    }
}
