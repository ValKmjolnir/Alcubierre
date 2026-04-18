#include "window.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "object/laser_beam.hpp"
#include "object/projectile.hpp"
#include "object/star.hpp"
#include "lighting_system.hpp"
#include "utils/shader_manager.hpp"

#include "raylib.h"
#include "raymath.h"

#include <vector>

int main() {
    game_window window(1600, 800, "Alcubierre Warp Drive");

    // TraceLogLevel(LOG_DEBUG);

    // Create 3D camera
    camera_3d camera(
        { -10.0f, 10.0f, -10.0f }, // position
        { 0.0f, 0.0f, 0.0f },      // target
        { 0.0f, 1.0f, 0.0f },      // up
        45.0f                      // fovy
    );

    // Create skybox (procedural gradient sky)
    skybox sky;

    // Create star (will become the primary light source)
    star main_star(
        { 0.0f, 10.0f, 300.0f }, // position
        255, 240, 200, 255       // color (warm white)
    );
    main_star.set_intensity(10.0f);

    // Create laser beam
    laser_beam beam(
        { 0.5f, 1.0f, 0.5f },   // start position (weapon)
        { 16.0f, 0.5f, 0.0f },  // end position (target)
        255, 150, 100, 255
    );
    beam.set_width(0.03f);
    beam.set_pulse_enabled(false);
    beam.set_pulse_speed(4.0f);
    beam.set_firing(true);

    std::vector<laser_beam> beams;
    for (int i = 0; i < 10; i++) {
        auto tmp = laser_beam(
            { 16.0f, -15.0f, -5.0f + i },
            { 0.0f, 1.0f, 0.0f },
            255, 155 + i * 10, i * 20, 255
        );
        tmp.set_width(0.03f);
        tmp.set_pulse_enabled(true);
        tmp.set_pulse_speed(2.0f);
        tmp.set_firing(true);
        beams.push_back(tmp);
    }

    // Enable bloom post-processing
    window.get_frame_graph().set_enable("bloom", true);
    window.get_bloom_renderer().set_bloom_threshold(0.85f);
    window.get_bloom_renderer().set_bloom_intensity(2.5f);
    window.get_bloom_renderer().set_bloom_blur_radius(15.0f);

    // Enable warp lens post-processing
    window.get_warp_renderer().set_velocity({ 0.0f, 0.0f, 0.0f });
    window.get_warp_renderer().set_bubble_radius(0.45f);
    window.get_warp_renderer().set_wall_thickness(0.075f);
    window.get_warp_renderer().set_exposure(1.0f);

    // Create projectile (orange)
    projectile proj1(
        { -3.0f, 2.0f, 0.0f },   // start position
        { 50.0f, 0.0f, 0.0f },   // velocity (moving right)
        255, 150, 0, 255         // color (orange)
    );
    proj1.set_radius(0.04f);
    proj1.set_lifetime(1.5f);
    proj1.set_trail_enabled(true);
    proj1.set_trail_length(5.0f);
    proj1.set_trail_color(255, 100, 0, 180);

    // Create second projectile (blue)
    projectile proj2(
        { -3.0f, 2.0f, 2.0f },
        { 30.0f, 2.5f, 0.0f },   // velocity with slight upward angle
        50, 150, 255, 255        // color (blue)
    );
    proj2.set_radius(0.04f);
    proj2.set_lifetime(2.0f);
    proj2.set_trail_enabled(true);
    proj2.set_trail_length(5.5f);
    proj2.set_trail_color(0, 100, 255, 160);

    bool draw_grid = true;
    bool show_text = true;
    bool enable_fxaa = false;
    bool enable_smaa = false;

    while (!window.should_close()) {
        window.begin_drawing();

        const float dt = GetFrameTime();

        // Update camera input
        camera.update(dt);

        // Toggle firing with space key
        if (IsKeyPressed(KEY_SPACE)) {
            draw_grid = !draw_grid;
        }
        if (IsKeyPressed(KEY_T)) {
            show_text = !show_text;
        }

        // Toggle bloom with B key
        if (IsKeyPressed(KEY_B)) {
            window.get_frame_graph().set_enable(
                "bloom",
                !window.get_frame_graph().enabled("bloom")
            );
        }

        if (IsKeyPressed(KEY_F)) {
            enable_fxaa = !enable_fxaa;
            window.get_frame_graph().set_enable("fxaa", enable_fxaa);
        }
        if (IsKeyPressed(KEY_G)) {
            enable_smaa = !enable_smaa;
            window.get_frame_graph().set_enable("smaa", enable_smaa);
        }

        // Adjust warp factor (bubble geometry)
        window.get_warp_renderer().update_warp_factor(dt);

        // Adjust velocity (beta = v/c) with UP/DOWN, direction with LEFT/RIGHT
        Vector3 vel = window.get_warp_renderer().get_velocity();
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
        // Write back the updated velocity
        vel.x = dirX * beta;
        vel.z = dirZ * beta;
        window.get_warp_renderer().set_velocity(vel);

        // Update view direction from camera
        Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target(), camera.position()));
        window.get_warp_renderer().set_view_direction(camForward);

        // Update laser beams
        beam.update(dt);
        for (auto& b : beams) {
            b.update(dt);
        }

        // Update projectiles
        proj1.update(dt);
        proj2.update(dt);

        // Reset projectiles if they become inactive
        if (!proj1.is_active()) {
            proj1.set_position({ -3.0f, 2.0f, 0.0f });
            proj1.set_active(true);
            proj1.set_age(0.0f);
        }
        if (!proj2.is_active()) {
            proj2.set_position({ -3.0f, 2.0f, 2.0f });
            proj2.set_active(true);
            proj2.set_age(0.0f);
        }

        // Begin render pass (renders to scene texture)
        window.begin_scene_pass();
        window.begin_mode_3d(camera.get_camera());

        // Draw skybox first (background)
        sky.draw(camera.get_camera());

        // Draw star (before other objects, will become light source)
        main_star.draw(camera, window.height(), 500.0f);

        // Draw grid on the ground plane
        if (draw_grid) {
            window.draw_grid(40, 8.0f);
        }

        // Draw cuboid
        window.draw_cube({ 0.0f, 1.0f, 0.0f }, 2.0f, 2.0f, 2.0f, 0, 50, 125);
        window.draw_cube({ 16.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, 125, 50, 0);
        window.draw_cube({ 16.0f, -15.0f, 0.0f }, 10.0f, 3.0f, 15.0f, 0, 100, 50);

        // Draw laser beams
        beam.draw();
        for (const auto& b : beams) {
            b.draw();
        }

        // Draw projectiles
        proj1.draw();
        proj2.draw();

        window.end_mode_3d();
        window.end_scene_pass();
        window.apply();

        if (show_text) {
            // Draw UI (on top of bloom)
            DrawFPS(10, 10);
            DrawText("Press SPACE to toggle grid", 10, 40, 16, WHITE);
            DrawText("Press T to toggle text", 10, 60, 16, WHITE);
            DrawText("Press B to toggle Bloom", 10, 80, 16, window.get_frame_graph().enabled("bloom") ? GREEN : GRAY);
            DrawText("Press F to toggle FXAA", 10, 100, 16, enable_fxaa ? GREEN : GRAY);
            DrawText("Press G to toggle SMAA", 10, 120, 16, enable_smaa ? GREEN : GRAY);

            // // Display current parameters
            char info_text[256];
            snprintf(info_text, 255, "Warp: %.2f | Beta (v/c): %.3f", window.get_warp_renderer().get_warp_factor(), beta);
            DrawText(info_text, 10, 140, 16, WHITE);

            char vel_text[256];
            const auto warp_vel = window.get_warp_renderer().get_velocity();
            snprintf(vel_text, 255, "Velocity: %.2f,%.2f,%.2f", warp_vel.x, warp_vel.y, warp_vel.z);
            DrawText(vel_text, 10, 160, 16, WHITE);

            char cam_text[256];
            snprintf(cam_text, 255, "Camera: %.2f,%.2f,%.2f", camForward.x, camForward.y, camForward.z);
            DrawText(cam_text, 10, 180, 16, WHITE);

            char light_info[64];
            snprintf(light_info, 64, "Active lights: %d", lighting_system::instance().active_light_count());
            DrawText(light_info, 10, 200, 16, YELLOW);

            DrawText("UP/DOWN = beta, LEFT/RIGHT = direction, PgUp/PgDn = warp", 10, 220, 16, GREEN);
        }
        window.end_drawing();
    }

    return 0;
}
