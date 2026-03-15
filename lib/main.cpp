#include "window.hpp"
#include "raylib.h"

int main() {
    game_window window(1600, 800, "Alcubierre");

    // 3D Camera setup
    Camera3D camera = { 0 };
    camera.position = { 5.0f, 5.0f, 5.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!window.should_close()) {
        window.begin_drawing();
        window.clear_background(25, 25, 25);

        // Begin 3D mode
        window.begin_mode_3d(camera);

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
