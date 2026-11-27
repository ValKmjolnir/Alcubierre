#pragma once

#include <raylib.h>

class game_window;

enum class start_action {
    none,
    start,
    settings,
    quit
};

// Title screen drawn over an empty starfield: big game title and three
// stacked buttons. Clicks only count when press and release land on the same
// button, matching the rest of the UI.
class start_screen {
private:
    game_window& window_;
    int pressed_button_ = -1;

    Rectangle button_rect(int index) const;
    int hit_button(const Vector2& point) const;

public:
    explicit start_screen(game_window& window);

    start_action update();
    void draw() const;
    void cancel_input() { pressed_button_ = -1; }
};
