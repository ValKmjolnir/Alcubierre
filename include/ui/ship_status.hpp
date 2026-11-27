#pragma once

#include <raylib.h>

class game_window;
struct ship_durability;

// Bottom-centre hull status: three bars (shield / armor / structure), each
// measured against its own maximum, with a short damage ghost behind the fill.
class ship_status {
private:
    game_window& window_;

    float fraction_[3] { 0.0f, 0.0f, 0.0f };
    float ghost_[3] { 0.0f, 0.0f, 0.0f };
    float hold_[3] { 0.0f, 0.0f, 0.0f };
    int current_value_[3] { 0, 0, 0 };
    int max_value_[3] { 0, 0, 0 };
    bool initialized_ = false;

    void update_layer(int index, float fraction, float dt, float hold_seconds);

public:
    explicit ship_status(game_window& window);

    void update(float dt, const ship_durability& current, const ship_durability& maximum);
    void draw() const;
    Rectangle bounds() const;
    bool contains(int x, int y) const;
};
