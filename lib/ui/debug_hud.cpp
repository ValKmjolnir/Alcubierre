#include <cstdio>

#include "ui/debug_hud.hpp"

void debug_hud::draw_right_padding(const char* text,
                                   int y,
                                   int size,
                                   Color color) {
    int text_width = MeasureText(text, size);
    DrawText(text, window.width() - text_width - 10, y, size, color);
}

void debug_hud::draw(float beta, const Vector3& cam_forward) {
    int fps = GetFPS();
    char fps_text[128];
    snprintf(fps_text, 127, "%d FPS", fps);
    draw_right_padding(fps_text, 10, 14, fps > 0.1 ? GREEN : RED);

    draw_right_padding("Press B to toggle Bloom", 30, 10, window.get_frame_graph().enabled("bloom") ? GREEN : GRAY);
    draw_right_padding("Press F to toggle FXAA", 45, 10, window.get_frame_graph().enabled("fxaa") ? GREEN : GRAY);
    draw_right_padding("Press G to toggle SMAA", 60, 10, window.get_frame_graph().enabled("smaa") ? GREEN : GRAY);

    // Display current parameters
    char info_text[256];
    snprintf(info_text, 255, "Warp: %.2f | Beta (v/c): %.3f", window.get_warp_renderer().get_warp_factor(), beta);
    draw_right_padding(info_text, 75, 10, WHITE);

    char vel_text[256];
    const auto warp_vel = window.get_warp_renderer().get_velocity();
    snprintf(vel_text, 255, "Velocity: %.2f,%.2f,%.2f", warp_vel.x, warp_vel.y, warp_vel.z);
    draw_right_padding(vel_text, 90, 10, WHITE);

    char cam_text[256];
    snprintf(cam_text, 255, "Camera: %.2f,%.2f,%.2f", cam_forward.x, cam_forward.y, cam_forward.z);
    draw_right_padding(cam_text, 105, 10, WHITE);

    draw_right_padding("UP/DOWN = beta, LEFT/RIGHT = direction, PgUp/PgDn = warp", 120, 10, WHITE);
}