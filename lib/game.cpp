#include <raylib.h>
#include <raymath.h>

#include "game.hpp"
#include "utils/game_config.hpp"
#include "scenes/fleet_scene.hpp"
#include "scenes/station_scene.hpp"
#include "scenes/warp_path.hpp"
#include "ship/ship.hpp"

namespace {

constexpr float kTargetLockSeconds = 0.8f;
constexpr float kAlignTurnDegPerSec = 30.0f;

constexpr float kWarpAccelSeconds = 1.3f;
constexpr float kWarpCruiseSeconds = 0.9f;
constexpr float kWarpDecelSeconds = 1.3f;
constexpr float kWarpSpoolSeconds =
    kWarpAccelSeconds + kWarpCruiseSeconds + kWarpDecelSeconds;
constexpr float kWarpArrivalSeconds = 3.5f;
constexpr float kWarpArrivalHoldSeconds = 0.5f;
constexpr float kWarpPeakFactor = 4.5f;
constexpr float kWarpPeakBeta = 0.9f;
constexpr float kWarpArrivalResidual = 0.35f;

constexpr float kWarpSpeedArea =
    kWarpAccelSeconds * 0.5f + kWarpCruiseSeconds + kWarpDecelSeconds * 0.5f;

float smooth01(float value) {
    value = Clamp(value, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

float warp_speed_shape(float elapsed) {
    if (elapsed <= 0.0f || elapsed >= kWarpSpoolSeconds) {
        return 0.0f;
    }
    if (elapsed < kWarpAccelSeconds) {
        return smooth01(elapsed / kWarpAccelSeconds);
    }
    if (elapsed < kWarpAccelSeconds + kWarpCruiseSeconds) {
        return 1.0f;
    }
    const float u = (elapsed - kWarpAccelSeconds - kWarpCruiseSeconds)
        / kWarpDecelSeconds;
    return 1.0f - smooth01(u);
}

float warp_progress(float elapsed) {
    if (elapsed <= 0.0f) {
        return 0.0f;
    }
    if (elapsed >= kWarpSpoolSeconds) {
        return 1.0f;
    }
    if (elapsed < kWarpAccelSeconds) {
        const float u = elapsed / kWarpAccelSeconds;
        const float u2 = u * u;
        return kWarpAccelSeconds * (u2 * u - 0.5f * u2 * u2)
            / kWarpSpeedArea;
    }
    if (elapsed < kWarpAccelSeconds + kWarpCruiseSeconds) {
        return (kWarpAccelSeconds * 0.5f
            + (elapsed - kWarpAccelSeconds)) / kWarpSpeedArea;
    }
    const float u = (elapsed - kWarpAccelSeconds - kWarpCruiseSeconds)
        / kWarpDecelSeconds;
    const float speed_integral = u - u * u * u + 0.5f * u * u * u * u;
    return (kWarpAccelSeconds * 0.5f + kWarpCruiseSeconds
        + kWarpDecelSeconds * speed_integral) / kWarpSpeedArea;
}

Vector3 ship_forward(const ship& s) {
    return Vector3Transform(
        Vector3 { 0.0f, 0.0f, -1.0f },
        MatrixRotateY(s.yaw_deg() * DEG2RAD));
}

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

    setup_scene();
    setup_ui();
    setup_post_processing();
}

game::~game() {
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
    aligning_ = false;
    warp_after_alignment_ = false;
    warp_stage_ = warp_stage::idle;
    warp_stage_time_ = 0.0f;

    active_scene_ = std::move(next);
    active_scene_->build(window_);

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
        const bool ui_owns_mouse = menu_.get_show_menu()
            || debug_window_.captures_mouse()
            || anchor_menu_.visible()
            || warp_stage_ != warp_stage::idle;

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            const Vector2 mouse = GetMousePosition();
            const warp_anchor* anchor = active_scene_->anchor();
            if (anchor_menu_.visible()) {
                anchor_menu_.close();
            } else if (warp_stage_ == warp_stage::idle
                       && !menu_.get_show_menu()
                       && !debug_window_.captures_mouse()
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
            grid_press_armed_ = over_grid
                && !menu_.get_show_menu()
                && !debug_window_.captures_mouse()
                && !anchor_menu_.visible()
                && warp_stage_ == warp_stage::idle;
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

        switch (anchor_menu_.update()) {
        case anchor_action::align_to:
            start_alignment(false);
            break;
        case anchor_action::warp_to:
            start_alignment(true);
            break;
        case anchor_action::none:
            break;
        }

        update_alignment(dt);
    }

    if (started_) {
        auto& warp = window_.get_warp_renderer();

        if (warp_stage_ == warp_stage::idle) {
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

    // The menu and the debug window own the mouse while the pointer is over them.
    const bool ui_owns_mouse = menu_.get_show_menu()
        || debug_window_.captures_mouse()
        || anchor_menu_.visible()
        || warp_stage_ != warp_stage::idle;
    const bool camera_accepts_mouse = !menu_.get_show_menu()
        && !debug_window_.captures_mouse()
        && !anchor_menu_.visible();

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

    update_warp_sequence(dt);

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

    camera_.update(dt, camera_accepts_mouse);
    lighting_system::instance().set_view_position(camera_.position());
    active_scene_->update(dt);

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

void game::start_alignment(bool warp_after) {
    if (warp_stage_ != warp_stage::idle) {
        return;
    }

    const warp_anchor* anchor = active_scene_->anchor();
    if (!anchor || !player_ship_) {
        return;
    }

    const Vector3 direction = Vector3Subtract(
        anchor->position(), player_ship_->position());
    if (fabsf(direction.x) < 1e-4f && fabsf(direction.z) < 1e-4f) {
        return;
    }

    align_target_yaw_ = atan2f(-direction.x, -direction.z) * RAD2DEG;
    aligning_ = true;
    warp_after_alignment_ = warp_after;
    TraceLog(LOG_INFO, "anchor: aligning to %.1f deg", align_target_yaw_);
}

void game::update_alignment(float dt) {
    if (!aligning_ || !player_ship_) {
        return;
    }

    if (!player_ship_->align_yaw_to(
            align_target_yaw_, kAlignTurnDegPerSec, dt)) {
        return;
    }

    aligning_ = false;
    const bool warp_after = warp_after_alignment_;
    warp_after_alignment_ = false;
    TraceLog(LOG_INFO, "anchor: alignment complete");

    if (warp_after) {
        TraceLog(LOG_INFO, "anchor: warp after alignment");
        start_warp_spool();
    }
}

void game::start_warp_spool() {
    if (warp_stage_ != warp_stage::idle) {
        return;
    }

    auto& warp = window_.get_warp_renderer();
    warp.set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    warp.set_warp_factor(0.0f);

    active_scene_->on_warp_start();

    if (!player_ship_) {
        return;
    }

    const warp_anchor* anchor = active_scene_->anchor();
    if (!anchor) {
        return;
    }
    const Vector3 to_anchor = Vector3Subtract(
        anchor->position(), player_ship_->position());
    const float anchor_distance = Vector3Length(to_anchor);
    if (anchor_distance < 0.001f) {
        return;
    }
    const Vector3 warp_direction = Vector3Scale(to_anchor, 1.0f / anchor_distance);

    player_ship_->clear_targets();
    camera_.set_target(player_ship_->position());
    warp_to_fleet_ = active_scene_->warp_to_fleet();

    warp_start_position_ = player_ship_->position();
    warp_end_position_ = Vector3Add(
        warp_start_position_,
        Vector3Scale(
            warp_direction,
            kWarpTravelDistance - kWarpArrivalDistance));

    warp_stage_ = warp_stage::spooling;
    warp_stage_time_ = 0.0f;
    anchor_menu_.close();
    TraceLog(LOG_INFO, "warp: spooling");
}

void game::update_warp_sequence(float dt) {
    if (warp_stage_ == warp_stage::idle || !player_ship_) {
        return;
    }

    auto& warp = window_.get_warp_renderer();
    warp_stage_time_ += dt;

    if (warp_stage_ == warp_stage::spooling) {
        const float elapsed = warp_stage_time_;
        const float progress = warp_progress(elapsed);
        const float speed = warp_speed_shape(elapsed);
        const float attack = smooth01(elapsed / kWarpAccelSeconds);
        const float envelope = attack
            * (kWarpArrivalResidual + (1.0f - kWarpArrivalResidual) * speed);
        warp.set_warp_factor(kWarpPeakFactor * envelope);
        warp.set_velocity(Vector3Scale(
            ship_forward(*player_ship_), kWarpPeakBeta * envelope));
        player_ship_->set_position(Vector3Add(
            warp_start_position_,
            Vector3Scale(
                Vector3Subtract(warp_end_position_, warp_start_position_),
                progress)));

        if (elapsed < kWarpSpoolSeconds) {
            return;
        }

        const float orbit_distance = camera_.distance();
        const float orbit_yaw = camera_.yaw();
        const float orbit_pitch = camera_.pitch();
        const bool to_fleet = warp_to_fleet_;
        const float arrival_yaw = player_ship_->yaw_deg();
        std::unique_ptr<game_scene> next = to_fleet
            ? std::unique_ptr<game_scene>(
                std::make_unique<fleet_scene>(lighting_system::instance()))
            : std::unique_ptr<game_scene>(
                std::make_unique<station_scene>(lighting_system::instance()));
        enter_scene(std::move(next));
        warp_to_fleet_ = false;
        warp_stage_ = warp_stage::arriving;
        warp_stage_time_ = 0.0f;

        if (player_ship_) {
            if (to_fleet) {
                player_ship_->set_yaw_deg(arrival_yaw);
            }
            camera_.set_target(player_ship_->position());
            camera_.set_orbit(orbit_distance, orbit_yaw, orbit_pitch);
            camera_.update(0.0f, false);
            warp.set_warp_factor(kWarpPeakFactor * kWarpArrivalResidual);
            warp.set_velocity(Vector3Scale(
                ship_forward(*player_ship_),
                kWarpPeakBeta * kWarpArrivalResidual));
        }
        TraceLog(LOG_INFO, "warp: jump");
        return;
    }

    const float fade_t = Clamp(
        (warp_stage_time_ - kWarpArrivalHoldSeconds)
            / (kWarpArrivalSeconds - kWarpArrivalHoldSeconds),
        0.0f, 1.0f);
    const float s = smooth01(fade_t);
    warp.set_warp_factor(
        kWarpPeakFactor * kWarpArrivalResidual * (1.0f - s));
    warp.set_velocity(Vector3Scale(
        ship_forward(*player_ship_),
        kWarpPeakBeta * kWarpArrivalResidual * (1.0f - s)));

    if (fade_t >= 1.0f) {
        warp_stage_ = warp_stage::idle;
        warp.set_warp_factor(0.0f);
        warp.set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
        TraceLog(LOG_INFO, "warp: arrival complete");
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
        const float lock_progress = target_locked_
            ? 1.0f
            : lock_elapsed_ / kTargetLockSeconds;
        if (marker_.contains(camera_, window_.width(), window_.height(),
                             *target_ship_, lock_progress, mouse)) {
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

    const bool warp_active = warp_stage_ != warp_stage::idle;

    clock::time_point scene_t0;
    if (show_hud) scene_t0 = clock::now();

    window_.begin_scene_pass();
    window_.begin_mode_3d(camera_.get_camera());

    sky_.draw(camera_.get_camera(), window_.get_warp_renderer().get_velocity());
    active_scene_->draw(camera_, window_.height());

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
            const float distance_km = Vector3Distance(
                player_ship_->position(), target_ship_->aim_point()) * 0.01f;
            const float lock_progress = target_locked_
                ? 1.0f
                : lock_elapsed_ / kTargetLockSeconds;
            marker_.draw(camera_, window_.width(), window_.height(),
                         *target_ship_, distance_km, lock_progress, target_locked_);
            target_status_.draw();
        }
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
