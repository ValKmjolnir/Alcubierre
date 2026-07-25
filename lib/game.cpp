#include <raylib.h>
#include <raymath.h>

#include "game.hpp"
#include "utils/game_config.hpp"

// ============================================================================
// Constructor
// ============================================================================

game::game()
    : window_(1600, 800, "Alcubierre Warp Drive")
    , scene_(lighting_system::instance())
    , main_star_(scene_.add<star>(
        Vector3 { 0.0f, 50.0f, 300.0f },
        255, 240, 200, 255))
    , camera_(
        Vector3 { -10.0f, 10.0f, -10.0f },
        Vector3 { 0.0f, 0.0f, 0.0f },
        Vector3 { 0.0f, 1.0f, 0.0f },
        45.0f)
    , sky_()
    , menu_(window_)
    , debug_hud_(window_)
    , input_()
    , beam_(
        Vector3 { 0.5f, 1.0f, 0.5f },
        Vector3 { 16.0f, 0.5f, 0.0f },
        255, 150, 100, 255)
    , proj1_(
        Vector3 { -3.0f, 2.0f, 0.0f },
        Vector3 { 50.0f, 0.0f, 0.0f },
        255, 150, 0, 255)
    , proj2_(
        Vector3 { -3.0f, 2.0f, 2.0f },
        Vector3 { 30.0f, 2.5f, 0.0f },
        50, 150, 255, 255)
{
    game_config::singleton().set_max_fps(80);

    Image icon = LoadImage("assets/logo/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    main_star_.set_intensity(10.0f);

    beam_.set_width(0.03f);
    beam_.set_pulse_enabled(false);
    beam_.set_pulse_speed(4.0f);
    beam_.set_firing(true);

    proj1_.set_radius(0.25f);
    proj1_.set_lifetime(1.5f);
    proj1_.set_trail_enabled(true);
    proj1_.set_trail_length(5.0f);
    proj1_.set_trail_color(255, 100, 0, 180);

    proj2_.set_radius(0.25f);
    proj2_.set_lifetime(2.0f);
    proj2_.set_trail_enabled(true);
    proj2_.set_trail_length(5.5f);
    proj2_.set_trail_color(0, 100, 255, 160);

    setup_beams();
    setup_ui();
    setup_post_processing();
}

// ============================================================================
// Setup helpers
// ============================================================================

void game::setup_beams() {
    beams_.reserve(10);
    for (int i = 0; i < 10; i++) {
        laser_beam b(
            Vector3 { 16.0f, -15.0f, -5.0f + static_cast<float>(i) },
            Vector3 { 0.0f, 1.0f, 0.0f },
            255, 155 + i * 10, i * 20, 255
        );
        b.set_width(0.03f);
        b.set_pulse_enabled(true);
        b.set_pulse_speed(2.0f);
        b.set_firing(true);
        beams_.push_back(std::move(b));
    }
}

void game::setup_ui() {
    menu_.add_new_textbox(40, 10, 100, 20, 18, WHITE, "Press ESC to toggle menu");
    menu_.add_new_textbox(40, 35, 100, 20, 18, GRAY, "Draw grid");
    menu_.add_new_checkbox(180, 35, 20,
        &game_config::get_enable_grid_draw,
        &game_config::set_enable_grid_draw);

    menu_.add_new_textbox(40, 60, 100, 20, 18, GRAY, "Debug hud");
    menu_.add_new_checkbox(180, 60, 20,
        &game_config::get_enable_debug_hud,
        &game_config::set_enable_debug_hud);

    menu_.add_new_textbox(40, 85, 100, 20, 18, GRAY, "Max FPS");
    menu_.add_new_slider(180, 85, 200, 20, 10, 240,
        &game_config::get_max_fps,
        &game_config::set_max_fps);

    menu_.add_new_textbox(40, 110, 100, 20, 18, GRAY, "Resolution");
    menu_.add_new_selector(180, 110, 200, 20);

    menu_.add_new_button(
        40, 135, 240, 30,
        button_operation::SET_TRUE,
        "exit game",
        &game_config::set_should_exit);
}

void game::setup_post_processing() {
    window_.get_frame_graph().set_enable("bloom", true);
    window_.get_bloom_renderer().set_bloom_threshold(0.85f);
    window_.get_bloom_renderer().set_bloom_intensity(2.5f);
    window_.get_bloom_renderer().set_bloom_blur_radius(15.0f);

    window_.get_warp_renderer().set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    window_.get_warp_renderer().set_bubble_radius(0.45f);
    window_.get_warp_renderer().set_wall_thickness(0.075f);
    window_.get_warp_renderer().set_exposure(1.0f);
}

// ============================================================================
// Per-frame methods
// ============================================================================

void game::handle_input(float dt) {
    // Toggle menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        menu_.set_show_menu(!menu_.get_show_menu());
        if (menu_.get_show_menu()) {
            input_.show_mouse();
        } else {
            input_.hide_mouse();
        }
    }

    // Toggle post-processing passes
    if (IsKeyPressed(KEY_B)) {
        auto& fg = window_.get_frame_graph();
        fg.set_enable("bloom", !fg.enabled("bloom"));
    }
    if (IsKeyPressed(KEY_F)) {
        auto& fg = window_.get_frame_graph();
        fg.set_enable("fxaa", !fg.enabled("fxaa"));
    }
    // Warp factor (PgUp / PgDn)
    auto& warp = window_.get_warp_renderer();
    float warp_factor = warp.get_warp_factor();
    float warp_step = 0.01f * (7.0f - warp_factor) * dt * 120.0f;
    if (IsKeyDown(KEY_PAGE_UP)) {
        warp_factor = fminf(warp_factor + warp_step, 6.0f);
    }
    if (IsKeyDown(KEY_PAGE_DOWN)) {
        warp_factor = fmaxf(warp_factor - warp_step, 0.0f);
    }
    warp.set_warp_factor(warp_factor);

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

void game::update(float dt) {
    camera_.update(dt);

    beam_.update(dt);
    for (auto& b : beams_) {
        b.update(dt);
    }

    proj1_.update(dt);
    proj2_.update(dt);

    if (!proj1_.is_active()) {
        proj1_.set_position(Vector3 { -3.0f, 2.0f, 0.0f });
        proj1_.set_active(true);
        proj1_.set_age(0.0f);
    }
    if (!proj2_.is_active()) {
        proj2_.set_position(Vector3 { -3.0f, 2.0f, 2.0f });
        proj2_.set_active(true);
        proj2_.set_age(0.0f);
    }

    // Sync view direction from camera
    Vector3 camForward = Vector3Normalize(
        Vector3Subtract(camera_.target(), camera_.position()));
    window_.get_warp_renderer().set_view_direction(camForward);
}

void game::render() {
    using clock = std::chrono::high_resolution_clock;

    bool show_hud = game_config::singleton().get_enable_debug_hud();
    window_.get_frame_graph().set_timing_enabled(show_hud);

    // Time 3D scene rendering
    clock::time_point scene_t0;
    if (show_hud) scene_t0 = clock::now();

    window_.begin_scene_pass();
    window_.begin_mode_3d(camera_.get_camera());

    sky_.draw(camera_.get_camera());
    main_star_.draw(camera_, window_.height(), 600.0f);

    if (game_config::singleton().get_enable_grid_draw()) {
        window_.draw_grid(40, 8.0f);
    }

    window_.draw_cube(Vector3 { 0.0f, 1.0f, 0.0f }, 2.0f, 2.0f, 2.0f, 0, 50, 125);
    window_.draw_cube(Vector3 { 16.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, 125, 50, 0);
    window_.draw_cube(Vector3 { 16.0f, -15.0f, 0.0f }, 10.0f, 3.0f, 15.0f, 0, 100, 50);

    beam_.draw();
    for (const auto& b : beams_) {
        b.draw();
    }

    proj1_.draw();
    proj2_.draw();

    window_.end_mode_3d();
    window_.end_scene_pass();

    double scene_ms = 0.0;
    if (show_hud) {
        scene_ms = std::chrono::duration<double, std::milli>(
            clock::now() - scene_t0).count();
    }

    window_.apply();

    if (show_hud) {
        Vector3 vel = window_.get_warp_renderer().get_velocity();
        float beta = Vector3Length(vel);
        Vector3 camForward = Vector3Normalize(
            Vector3Subtract(camera_.target(), camera_.position()));
        debug_hud_.draw(beta, camForward);

        auto timings = window_.get_frame_graph().get_pass_timings();
        timings["scene"] = scene_ms;
        debug_hud_.draw_timings(timings);
    }

    menu_.draw();
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
