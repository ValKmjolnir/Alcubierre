#pragma once

#include <raylib.h>

#include "window.hpp"

class debug_hud {
private:
    bool show_text_ = false;

public:
    bool show_text() const { return show_text_; }
    void check_f3_toggle();
    void draw(float beta, const Vector3& cam_forward, game_window& window);
};