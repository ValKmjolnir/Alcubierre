#pragma once

#include <raylib.h>

class game_window;
class ship;

// Right-hand target card: name, distance, lock state and the three durability
// bars for whichever ship the player has locked.
class target_status {
private:
    game_window& window_;

    const ship* target_ = nullptr;
    float distance_km_ = 0.0f;
    float progress_ = 0.0f;
    bool locked_ = false;

    float fraction_[3] { 0.0f, 0.0f, 0.0f };
    float ghost_[3] { 0.0f, 0.0f, 0.0f };
    float hold_[3] { 0.0f, 0.0f, 0.0f };
    int current_value_[3] { 0, 0, 0 };
    int max_value_[3] { 0, 0, 0 };
    bool initialized_ = false;

public:
    explicit target_status(game_window& window);

    void update(float dt, const ship* target, float distance_km,
                float progress, bool locked);
    void draw() const;
    bool contains(int x, int y) const;
};
