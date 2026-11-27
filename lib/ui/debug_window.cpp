#include <cstdio>

#include "ui/debug_window.hpp"
#include "ui/text_wrap.hpp"
#include "utils/game_config.hpp"
#include "window.hpp"

debug_window::debug_window(game_window& window) :
    panel(window.width() - 400, 40, 380, 360, Color { 8, 12, 18, 215 }),
    window(window) {
    panel.set_title("Debug");
    panel.set_draggable(true);
    panel.set_border(Color { 0x00, 0xdd, 0xff, 150 });

    panel.add_new_textbox(12, 36, 100, 20, 16, WHITE, "Bloom (B)");
    panel.add_new_checkbox(120, 36, 18,
        &game_config::get_enable_bloom,
        &game_config::set_enable_bloom);

    panel.add_new_textbox(12, 60, 100, 20, 16, WHITE, "FXAA (F)");
    panel.add_new_checkbox(120, 60, 18,
        &game_config::get_enable_fxaa,
        &game_config::set_enable_fxaa);

    fps_ = &panel.add_new_textbox(12, 92, 200, 20, 16, WHITE, "");
    warp_ = &panel.add_new_textbox(12, 114, 350, 20, 16, WHITE, "");
    velocity_ = &panel.add_new_textbox(12, 136, 350, 20, 16, WHITE, "");
    camera_ = &panel.add_new_textbox(12, 158, 350, 20, 16, WHITE, "");
    panel.add_new_textbox(
        12, 180, 356, 20, 12, WHITE,
        wrap_text("UP/DOWN = beta, LEFT/RIGHT = direction, PgUp/PgDn = warp, click = lock target",
                  12, 356).c_str());
    timings_ = &panel.add_new_textbox(12, 232, 356, 110, 12, WHITE, "");

    panel.add_new_button(380 - 26, 3, 20, 20,
        button_operation::SET_FALSE,
        "x",
        &game_config::set_enable_debug_hud);
}

void debug_window::update(float beta,
                          const Vector3& cam_forward,
                          const std::unordered_map<std::string, double>& timings) {
    char buf[512];

    const int fps = GetFPS();
    snprintf(buf, sizeof(buf), "%d FPS", fps);
    fps_->set_text(buf);
    fps_->set_color(fps > 0 ? GREEN : RED);

    snprintf(buf, sizeof(buf), "Warp: %.2f | Beta (v/c): %.3f",
             window.get_warp_renderer().get_warp_factor(), beta);
    warp_->set_text(buf);

    const auto vel = window.get_warp_renderer().get_velocity();
    snprintf(buf, sizeof(buf), "Velocity: %.2f,%.2f,%.2f", vel.x, vel.y, vel.z);
    velocity_->set_text(buf);

    snprintf(buf, sizeof(buf), "Camera: %.2f,%.2f,%.2f",
             cam_forward.x, cam_forward.y, cam_forward.z);
    camera_->set_text(buf);

    if (timings.empty()) {
        timings_->set_text("Pass timings: (none)");
        return;
    }

    std::string text = "Pass timings (ms):";
    double total = 0.0;
    char line[96];
    for (const auto& [name, ms] : timings) {
        snprintf(line, sizeof(line), "\n  %s: %.2f", name.c_str(), ms);
        text += line;
        total += ms;
    }
    snprintf(line, sizeof(line), "\n  Total: %.2f ms", total);
    text += line;
    timings_->set_text(text);
}

void debug_window::draw() {
    panel.clamp_inside_screen();
    panel.update_input();
    panel.draw();
}
