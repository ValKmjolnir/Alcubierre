#include "window.hpp"
#include "camera.hpp"
#include "laser_beam.hpp"
#include "raylib.h"
#include "raymath.h"

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

    // Create laser beam
    laser_beam beam(
        { 0.0f, 2.0f, 0.0f },   // start position (weapon)
        { 15.0f, 1.0f, 0.0f },  // end position (target)
        255, 50, 50, 255        // color (red with alpha)
    );
    beam.set_width(0.015f);
    beam.set_intensity(1.2f);
    beam.set_jitter_enabled(true);
    beam.set_jitter_amount(0.08f);
    beam.set_pulse_enabled(true);
    beam.set_pulse_speed(15.0f);
    beam.set_firing(true);

    // Second laser beam (blue)
    laser_beam beam2(
        { -5.0f, 2.0f, 3.0f },
        { 10.0f, 0.5f, -5.0f },
        50, 100, 255, 255
    );
    beam2.set_width(0.02f);
    beam2.set_firing(true);

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

        window.begin_drawing();
        window.clear_background(25, 25, 25);

        // Begin 3D mode
        window.begin_mode_3d(camera.get_camera());

        // Draw grid on the ground plane
        window.draw_grid(1.0f, 40);

        // Draw a cuboid at the origin
        window.draw_cube({ 0.0f, 1.0f, 0.0f }, 2.0f, 2.0f, 2.0f, 0, 128, 255);

        // Draw another cuboid
        window.draw_cube({ 16.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, 255, 128, 0);

        // Draw laser beams
        beam.draw();
        beam2.draw();

        // End 3D mode
        window.end_mode_3d();

        // Draw UI
        DrawFPS(10, 10);
        DrawText("Press SPACE to toggle laser firing", 10, 40, 20, WHITE);
        DrawText(beam.is_firing() ? "LASER: FIRING" : "LASER: OFF", 10, 70, 20,
                 beam.is_firing() ? RED : DARKGRAY);

        window.end_drawing();
    }

    return 0;
}
