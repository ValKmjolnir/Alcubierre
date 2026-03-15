#include "window.hpp"
#include "camera.hpp"
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

    while (!window.should_close()) {
        const float dt = GetFrameTime();
        
        window.begin_drawing();
        window.clear_background(25, 25, 25);

        // Update camera input
        camera.update(dt);

        // Begin 3D mode
        window.begin_mode_3d(camera.get_camera());

        // Draw grid on the ground plane
        window.draw_grid(1.0f, 20);

        // Draw a cuboid at the origin
        window.draw_cube({ 0.0f, 1.0f, 0.0f }, 2.0f, 2.0f, 2.0f, 0, 128, 255);

        // Draw another cuboid
        window.draw_cube({ 3.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, 255, 128, 0);

        // End 3D mode
        window.end_mode_3d();

        // Draw FPS
        DrawFPS(10, 10);

        window.end_drawing();
    }

    return 0;
}
