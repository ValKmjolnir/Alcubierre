#pragma once

#include <raylib.h>

#include <unordered_map>

#include "ui/ui_window.hpp"

class game_window;
class textbox;

class debug_window {
private:
    ui_window panel;
    game_window& window;

    textbox* fps_ = nullptr;
    textbox* warp_ = nullptr;
    textbox* velocity_ = nullptr;
    textbox* camera_ = nullptr;
    textbox* timings_ = nullptr;

public:
    debug_window(game_window& window);

    void set_visible(bool flag) { panel.set_visible(flag); }
    bool get_visible() const { return panel.get_visible(); }
    bool captures_mouse() const { return panel.captures_mouse(); }

    void update(float beta,
                const Vector3& cam_forward,
                const std::unordered_map<std::string, double>& timings);
    void draw();
};
