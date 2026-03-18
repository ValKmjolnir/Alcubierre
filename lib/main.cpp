#include "window.hpp"
#include "camera.hpp"
#include "laser_beam.hpp"
#include "projectile.hpp"
#include "skybox.hpp"
#include "raylib.h"
#include "raymath.h"

#include <vector>

int main() {
    game_window window(1600, 800, "Alcubierre");

    // Create 3D camera
    camera_3d camera(
        { 5.0f, 5.0f, 5.0f },  // position
        { 0.0f, 0.0f, 0.0f },  // target
        { 0.0f, 1.0f, 0.0f },  // up
        45.0f                  // fovy
    );
    camera.set_mode_free();

    // Create skybox (procedural gradient sky)
    skybox sky;

    // Create laser beam
    laser_beam beam(
        { 0.0f, 1.5f, 0.0f },   // start position (weapon)
        { 16.0f, 1.0f, 0.0f },  // end position (target)
        255, 50, 50, 255        // color (red with alpha)
    );
    beam.set_width(0.05f);
    beam.set_pulse_enabled(true);
    beam.set_pulse_speed(2.0f);
    beam.set_firing(true);

    // Second laser beam (blue)
    laser_beam beam2(
        { 0.5f, 1.0f, 0.5f },
        { 16.0f, 0.5f, 0.0f },
        50, 100, 255, 255
    );
    beam2.set_width(0.07f);
    beam2.set_firing(true);

    std::vector<laser_beam> beams;
    for (int i = 0; i < 10; i++) {
        auto tmp = laser_beam(
            { 16.0f, -15.0f, -5.0f + i },
            { 0.0f, 1.0f, 0.0f },
            i, i * 10, i * 10, 255
        );
        tmp.set_width(0.05f);
        tmp.set_pulse_enabled(true);
        tmp.set_pulse_speed(2.0f);
        tmp.set_firing(true);
        beams.push_back(tmp);
    }

    // Create projectile (orange)
    projectile proj1(
        { -3.0f, 2.0f, 0.0f },   // start position
        { 15.0f, 0.0f, 0.0f },   // velocity (moving right)
        255, 150, 0, 255         // color (orange)
    );
    proj1.set_radius(0.05f);
    proj1.set_lifetime(1.5f);
    proj1.set_trail_enabled(true);
    proj1.set_trail_length(2.0f);
    proj1.set_trail_color(255, 100, 0, 180);

    // Create second projectile (blue)
    projectile proj2(
        { -3.0f, 2.0f, 2.0f },
        { 12.0f, 2.0f, 0.0f },   // velocity with slight upward angle
        50, 150, 255, 255        // color (blue)
    );
    proj2.set_radius(0.05f);
    proj2.set_lifetime(2.0f);
    proj2.set_trail_enabled(true);
    proj2.set_trail_length(1.5f);
    proj2.set_trail_color(0, 100, 255, 160);

    while (!window.should_close()) {
        const float dt = GetFrameTime();

        // Update camera input
        camera.update(dt);

        // Toggle firing with space key
        if (IsKeyPressed(KEY_SPACE)) {
            const bool new_state = !beam.is_firing();
            beam.set_firing(new_state);
            beam2.set_firing(new_state);
            TraceLog(LOG_INFO, "SPACE pressed - firing state: %s", new_state ? "ON" : "OFF");
        }

        // Update laser beams
        beam.update(dt);
        beam2.update(dt);
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

        window.begin_drawing();
        window.clear_background(25, 25, 25);

        // Begin 3D mode
        window.begin_mode_3d(camera.get_camera());

        // Draw skybox first (background)
        sky.draw(camera.get_camera());

        // Draw grid on the ground plane
        window.draw_grid(1.0f, 40);

        // Draw a cuboid at the origin
        window.draw_cube({ 0.0f, 1.0f, 0.0f }, 2.0f, 2.0f, 2.0f, 0, 128, 255);

        // Draw another cuboid
        window.draw_cube({ 16.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, 255, 128, 0);

        window.draw_cube({ 16.0f, -15.0f, 0.0f }, 10.0f, 3.0f, 15.0f, 0, 255, 128);

        // Draw laser beams
        beam.draw();
        beam2.draw();
        for (const auto& b : beams) {
            b.draw();
        }

        // Draw projectiles
        proj1.draw();
        proj2.draw();

        // End 3D mode
        window.end_mode_3d();

        // Draw UI
        DrawFPS(10, 10);
        DrawText("Press SPACE to toggle laser firing", 10, 40, 20, WHITE);
        DrawText("Projectiles auto-fire and reset", 10, 65, 20, WHITE);
        DrawText(beam.is_firing() ? "LASER: FIRING" : "LASER: OFF", 10, 95, 20,
                 beam.is_firing() ? RED : DARKGRAY);

        window.end_drawing();
    }

    return 0;
}
