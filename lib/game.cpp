#include <raylib.h>
#include <raymath.h>

#include "game.hpp"
#include "utils/game_config.hpp"
#include "object/star.hpp"
#include "ship/ship.hpp"
#include "ship/ship_class.hpp"

namespace {

constexpr float kTargetLockSeconds = 0.8f;

}

// ============================================================================
// Constructor
// ============================================================================

game::game()
    : window_(1600, 800, "Alcubierre Warp Drive")
    , scene_(lighting_system::instance())
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
    , input_()
{
    game_config::singleton().set_max_fps(80);

    Image icon = LoadImage("assets/logo/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    setup_scene();
    setup_ui();
    setup_post_processing();
}

game::~game() {
    lighting_system::instance().remove(&ambient_);
}

// ============================================================================
// Scene setup — all objects registered via scene_
// ============================================================================

void game::setup_scene() {
    // The capital ship still sits well beyond the near fleet, so the far plane
    // has to clear raylib's 1000-unit default; near is raised to keep 24-bit
    // depth usable.
    window_.set_clip_planes(0.25f, 10000.0f);

    // Fleet layout, shared by the hulls and the laser attach points.
    const Vector3 player_pos { 0.0f, 0.0f, 0.0f };
    const Vector3 escort_pos { -46.0f, -4.5f, -40.0f };
    const float escort_yaw = -60.0f;
    const Vector3 titan_pos { 110.0f, 40.0f, -1500.0f };
    const float titan_yaw = 105.0f;

    // --- Ambient light ---
    ambient_.set_sky_color({ 0.40f, 0.44f, 0.52f });
    ambient_.set_ground_color({ 0.10f, 0.11f, 0.13f });
    ambient_.set_intensity(1.0f);
    lighting_system::instance().add(&ambient_);

    // --- Star ---
    auto& main_star = scene_.add<star>(
        Vector3 { 0.0f, 50.0f, 600.0f }, 255, 190, 155, 255);
    main_star.set_intensity(10.0f);
    main_star.set_scale(1000.0f);

    // --- Fleet ---
    // 1 unit = 10 m: the player craft and the escort are 15-unit corvettes,
    // the capital ship is a 10 km titan 15 km out so both frame together.
    const auto ship_class_of = [](const char* id) -> const ship_class_def& {
        const ship_class_def* def = find_ship_class(id);
        if (!def) {
            TraceLog(LOG_FATAL, "unknown ship class %s", id);
        }
        return *def;
    };
    const auto weapon_of = [](const char* id) -> const weapon_def& {
        const weapon_def* def = find_weapon(id);
        if (!def) {
            TraceLog(LOG_FATAL, "unknown weapon %s", id);
        }
        return *def;
    };

    auto& player = scene_.add<ship>(
        window_, ship_class_of("corvette_glb"), player_pos, 0.0f);
    player_ship_ = &player;
    auto& escort = scene_.add<ship>(
        window_, ship_class_of("corvette_glb"), escort_pos, escort_yaw);
    auto& titan = scene_.add<ship>(
        window_, ship_class_of("titan_blockout"), titan_pos, titan_yaw);
    hostiles_.push_back(&escort);
    hostiles_.push_back(&titan);

    // Player: three of the hull's sockets, the remaining four stay empty.
    player.install_weapon(0, weapon_of("pulse_laser_s"));
    player.install_weapon(1, weapon_of("autocannon_s"));
    player.install_weapon(2, weapon_of("shield_restorer_s"));

    // Escort: five dorsal laser sockets, two ventral point-defence sockets.
    for (int i = 0; i < 5; i++) {
        escort.install_weapon(i, weapon_of("pulse_laser_s"));
    }
    escort.install_weapon(5, weapon_of("point_defence_s"));
    escort.install_weapon(6, weapon_of("point_defence_s"));

    // Titan: spinal lance, two dorsal barbettes, two ventral batteries and
    // four point-defence clusters.
    titan.install_weapon(0, weapon_of("spinal_lance_c"));
    titan.install_weapon(1, weapon_of("beam_lance_c"));
    titan.install_weapon(2, weapon_of("beam_lance_c"));
    titan.install_weapon(3, weapon_of("flak_battery_c"));
    titan.install_weapon(4, weapon_of("flak_battery_c"));
    for (int i = 5; i < 9; i++) {
        titan.install_weapon(i, weapon_of("point_defence_c"));
    }

    player.aim_at(escort);
    escort.aim_at(player);
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

void game::handle_input(float dt) {
    auto& config = game_config::singleton();

    if (IsKeyPressed(KEY_ESCAPE)) {
        menu_.set_show_menu(!menu_.get_show_menu());
    }
    if (IsKeyPressed(KEY_F1)) {
        config.set_enable_debug_hud(!config.get_enable_debug_hud());
    }
    if (IsKeyPressed(KEY_B)) {
        config.set_enable_bloom(!config.get_enable_bloom());
    }
    if (IsKeyPressed(KEY_F)) {
        config.set_enable_fxaa(!config.get_enable_fxaa());
    }

    debug_window_.set_visible(config.get_enable_debug_hud());

    input_.show_mouse();

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
        const bool ui_owns_mouse = menu_.get_show_menu() || debug_window_.captures_mouse();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            press_position_ = GetMousePosition();
            const int mouse_x = int(press_position_.x);
            const int mouse_y = int(press_position_.y);
            const bool over_grid = grid_button_.contains(status_.bounds(), mouse_x, mouse_y);
            grid_press_armed_ = over_grid
                && !menu_.get_show_menu()
                && !debug_window_.captures_mouse();
            pick_armed_ = !ui_owns_mouse
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
    }

    if (started_) {
        // Warp factor (PgUp / PgDn)
        auto& warp = window_.get_warp_renderer();
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

void game::update(float dt) {
    auto& config = game_config::singleton();
    auto& frame_graph = window_.get_frame_graph();
    frame_graph.set_enable("bloom", config.get_enable_bloom());
    frame_graph.set_enable("fxaa", config.get_enable_fxaa());

    // The menu and the debug window own the mouse while the pointer is over them.
    const bool ui_owns_mouse = menu_.get_show_menu() || debug_window_.captures_mouse();

    if (!started_) {
        if (player_ship_) {
            camera_.set_target(player_ship_->position());
        }
        camera_.update(dt, false);
        lighting_system::instance().set_view_position(camera_.position());

        const Vector3 cam_forward = Vector3Normalize(
            Vector3Subtract(camera_.target(), camera_.position()));
        auto& warp = window_.get_warp_renderer();
        warp.set_view_direction(cam_forward);
        warp.set_fov_y(camera_.fovy());
        return;
    }

    if (player_ship_) {
        camera_.set_target(player_ship_->position());
        status_.update(dt, player_ship_->health(), player_ship_->max_health());
    }

    update_target_lock(dt);

    float distance_km = 0.0f;
    if (target_ship_ && player_ship_) {
        distance_km = Vector3Distance(
            player_ship_->position(), target_ship_->aim_point()) * 0.01f;
    }
    const float lock_progress = target_locked_
        ? 1.0f
        : lock_elapsed_ / kTargetLockSeconds;
    target_status_.update(dt, target_ship_, distance_km, lock_progress, target_locked_);

    camera_.update(dt, !ui_owns_mouse);
    lighting_system::instance().set_view_position(camera_.position());
    scene_.update_all(dt);

    Vector3 camForward = Vector3Normalize(
        Vector3Subtract(camera_.target(), camera_.position()));
    auto& warp = window_.get_warp_renderer();
    warp.set_view_direction(camForward);
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
    if (target_locked_) {
        return;
    }

    lock_elapsed_ += dt;
    if (lock_elapsed_ >= kTargetLockSeconds) {
        lock_elapsed_ = kTargetLockSeconds;
        target_locked_ = true;
    }
}

void game::set_target(ship* target) {
    if (target == target_ship_) {
        return;
    }
    target_ship_ = target;
    lock_elapsed_ = 0.0f;
    target_locked_ = false;
}

void game::clear_target() {
    target_ship_ = nullptr;
    lock_elapsed_ = 0.0f;
    target_locked_ = false;
}

void game::select_target_at_mouse() {
    const Ray ray = GetScreenToWorldRayEx(
        GetMousePosition(), camera_.get_camera(), window_.width(), window_.height());
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

    set_target(best);
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
        return;
    }

    clock::time_point scene_t0;
    if (show_hud) scene_t0 = clock::now();

    window_.begin_scene_pass();
    window_.begin_mode_3d(camera_.get_camera());

    sky_.draw(camera_.get_camera(), window_.get_warp_renderer().get_velocity());
    scene_.draw_all(camera_, window_.height());

    if (game_config::singleton().get_enable_grid_draw() && player_ship_) {
        grid_.draw(player_ship_->position());
    }

    window_.end_mode_3d();
    window_.end_scene_pass();

    auto& bloom_pass = window_.get_bloom_renderer();
    bloom_pass.begin_flare_pass();
    Vector2 flare_origin = scene_.draw_flares(
        camera_, bloom_pass.flare_width(), bloom_pass.flare_height(), window_.height());
    bloom_pass.end_flare_pass();
    bloom_pass.set_streak_origin(flare_origin);


    double scene_ms = 0.0;
    if (show_hud) {
        scene_ms = std::chrono::duration<double, std::milli>(
            clock::now() - scene_t0).count();
    }

    window_.apply();

    status_.draw();
    grid_button_.draw(status_.bounds());

    if (target_ship_ && player_ship_) {
        const float distance_km = Vector3Distance(
            player_ship_->position(), target_ship_->aim_point()) * 0.01f;
        const float lock_progress = target_locked_
            ? 1.0f
            : lock_elapsed_ / kTargetLockSeconds;
        marker_.draw(camera_, window_.width(), window_.height(),
                     *target_ship_, distance_km, lock_progress, target_locked_);
        target_status_.draw();
    }

    menu_.draw(!debug_window_.captures_mouse());

    if (show_hud) {
        Vector3 vel = window_.get_warp_renderer().get_velocity();
        float beta = Vector3Length(vel);
        Vector3 camForward = Vector3Normalize(
            Vector3Subtract(camera_.target(), camera_.position()));

        auto timings = window_.get_frame_graph().get_pass_timings();
        timings["scene"] = scene_ms;
        debug_window_.update(beta, camForward, timings);
        debug_window_.draw();
    }
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
