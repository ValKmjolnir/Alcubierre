#include <cstdio>

#include "ui/debug_hud.hpp"

void debug_hud::check_f3_toggle() {
    if (IsKeyPressed(KEY_F3)) {
        show_text_ = !show_text_;
    }
}

void debug_hud::draw(float beta, const Vector3& cam_forward, game_window& window) {
    int fps = GetFPS();
    char fps_text[128];
    snprintf(fps_text, 127, "%d FPS", fps);
    DrawText(fps_text, 10, 10, 14, fps > 0.1 ? GREEN : RED);

    DrawText("Press SPACE to toggle grid", 10, 30, 10, WHITE);
    DrawText("Press F3 to toggle debug HUD", 10, 45, 10, WHITE);
    DrawText("Press B to toggle Bloom", 10, 60, 10, window.get_frame_graph().enabled("bloom") ? GREEN : GRAY);
    DrawText("Press F to toggle FXAA", 10, 75, 10, window.get_frame_graph().enabled("fxaa") ? GREEN : GRAY);
    DrawText("Press G to toggle SMAA", 10, 90, 10, window.get_frame_graph().enabled("smaa") ? GREEN : GRAY);

    // Display current parameters
    char info_text[256];
    snprintf(info_text, 255, "Warp: %.2f | Beta (v/c): %.3f", window.get_warp_renderer().get_warp_factor(), beta);
    DrawText(info_text, 10, 105, 10, WHITE);

    char vel_text[256];
    const auto warp_vel = window.get_warp_renderer().get_velocity();
    snprintf(vel_text, 255, "Velocity: %.2f,%.2f,%.2f", warp_vel.x, warp_vel.y, warp_vel.z);
    DrawText(vel_text, 10, 120, 10, WHITE);

    char cam_text[256];
    snprintf(cam_text, 255, "Camera: %.2f,%.2f,%.2f", cam_forward.x, cam_forward.y, cam_forward.z);
    DrawText(cam_text, 10, 135, 10, WHITE);

    DrawText("UP/DOWN = beta, LEFT/RIGHT = direction, PgUp/PgDn = warp", 10, 165, 10, WHITE);
}