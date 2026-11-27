#include <raylib.h>

#include <cstdio>

#include "ui/target_status.hpp"
#include "ui/durability_style.hpp"
#include "ship/ship.hpp"
#include "ship/ship_class.hpp"
#include "window.hpp"

namespace {

constexpr int kPanelWidth = 268;
constexpr int kPanelHeight = 140;
constexpr int kMargin = 24;
constexpr int kPadding = 10;
constexpr int kLabelWidth = 64;
constexpr int kValueWidth = 88;
constexpr int kRowHeight = 13;
constexpr int kRowGap = 6;
constexpr int kBarsTop = 68;
constexpr int kStatusBarClearance = 104;

constexpr float kGhostHoldSeconds = 0.35f;
constexpr float kGhostDecayPerSecond = 0.9f;

const Color kBackground { 10, 14, 20, 205 };
const Color kBorder { 0x00, 0xdd, 0xff, 90 };
const Color kHostile { 0xff, 0x6a, 0x5a, 255 };
const Color kMuted { 130, 140, 152, 255 };
const Color kText { 200, 210, 220, 255 };
const Color kAccent { 0x00, 0xdd, 0xff, 255 };

const char* size_label(ship_size size) {
    switch (size) {
    case ship_size::frigate: return "FRIGATE";
    case ship_size::cruiser: return "CRUISER";
    case ship_size::battleship: return "BATTLESHIP";
    default: return "TITAN";
    }
}

void format_distance(char* out, size_t size, float km) {
    if (km < 10.0f) {
        snprintf(out, size, "%.2f km", km);
    } else {
        snprintf(out, size, "%.1f km", km);
    }
}

void draw_right(const char* text, int right_x, int y, int font_size, Color color) {
    DrawText(text, right_x - MeasureText(text, font_size), y, font_size, color);
}

}

target_status::target_status(game_window& window) : window_(window) {}

void target_status::update(float dt, const ship* target, float distance_km,
                           float progress, bool locked) {
    if (target != target_) {
        target_ = target;
        initialized_ = false;
    }

    distance_km_ = distance_km;
    progress_ = progress;
    locked_ = locked;

    if (!target_) {
        return;
    }

    const ship_durability& current = target_->health();
    const ship_durability& maximum = target_->max_health();
    const float current_values[3] = { current.shield, current.armor, current.structure };
    const float maximum_values[3] = { maximum.shield, maximum.armor, maximum.structure };

    for (int i = 0; i < 3; i++) {
        fraction_[i] = durability_fraction(current_values[i], maximum_values[i]);
        current_value_[i] = int(current_values[i]);
        max_value_[i] = int(maximum_values[i]);

        if (!initialized_) {
            ghost_[i] = fraction_[i];
            hold_[i] = 0.0f;
        }

        if (fraction_[i] < ghost_[i]) {
            if (hold_[i] > 0.0f) {
                hold_[i] -= dt;
            } else {
                ghost_[i] = fmaxf(fraction_[i], ghost_[i] - dt * kGhostDecayPerSecond);
            }
        } else {
            ghost_[i] = fraction_[i];
            hold_[i] = kGhostHoldSeconds;
        }
    }
    initialized_ = true;
}

void target_status::draw() const {
    if (!target_) {
        return;
    }

    const int x = window_.width() - kPanelWidth - kMargin;
    const int y = window_.height() - kPanelHeight - kStatusBarClearance;

    DrawRectangle(x, y, kPanelWidth, kPanelHeight, kBackground);
    DrawRectangleLines(x, y, kPanelWidth, kPanelHeight, kBorder);
    DrawRectangle(x, y, 3, kPanelHeight, kHostile);

    DrawText("TARGET", x + kPadding, y + 8, 10, kMuted);
    draw_right("HOSTILE", x + kPanelWidth - kPadding, y + 8, 10, kHostile);

    DrawText(target_->ship_class().name, x + kPadding, y + 21, 18, WHITE);

    char distance[32];
    format_distance(distance, sizeof(distance), distance_km_);
    draw_right(distance, x + kPanelWidth - kPadding, y + 25, 13, kAccent);

    DrawText(size_label(target_->ship_class().size), x + kPadding, y + 44, 10, kMuted);

    char lock_state[32];
    if (locked_) {
        snprintf(lock_state, sizeof(lock_state), "LOCKED");
    } else {
        snprintf(lock_state, sizeof(lock_state), "LOCKING %d%%", int(progress_ * 100.0f));
    }
    draw_right(lock_state, x + kPanelWidth - kPadding, y + 44, 10,
               locked_ ? kAccent : Color { 255, 180, 88, 255 });

    if (locked_) {
        DrawRectangle(x + kPadding, y + 60, kPanelWidth - kPadding * 2, 1,
                      Color { 0x00, 0xdd, 0xff, 60 });
    } else {
        const int bar_width = kPanelWidth - kPadding * 2;
        DrawRectangle(x + kPadding, y + 59, bar_width, 3, Color { 26, 32, 42, 230 });
        DrawRectangle(x + kPadding, y + 59, int(bar_width * progress_), 3, kAccent);
    }

    const int bar_x = x + kPadding + kLabelWidth;
    const int bar_width = kPanelWidth - kPadding * 2 - kLabelWidth - kValueWidth;

    for (int i = 0; i < 3; i++) {
        const int row_y = y + kBarsTop + i * (kRowHeight + kRowGap);
        const Color color = durability_layer_color(i);

        DrawText(durability_layer_label(i), x + kPadding, row_y, 10, color);
        DrawRectangle(bar_x, row_y - 1, bar_width, kRowHeight, Color { 26, 32, 42, 230 });

        const int ghost_width = int(bar_width * ghost_[i]);
        if (ghost_width > 0) {
            DrawRectangle(bar_x, row_y - 1, ghost_width, kRowHeight, Fade(color, 0.35f));
        }

        const int fill_width = int(bar_width * fraction_[i]);
        if (fill_width > 0) {
            DrawRectangle(bar_x, row_y - 1, fill_width, kRowHeight, color);
        }

        char value[32];
        snprintf(value, sizeof(value), "%d/%d", current_value_[i], max_value_[i]);
        draw_right(value, x + kPanelWidth - kPadding, row_y, 10, kText);
    }
}

bool target_status::contains(int x, int y) const {
    if (!target_) {
        return false;
    }
    const int left = window_.width() - kPanelWidth - kMargin;
    const int top = window_.height() - kPanelHeight - kStatusBarClearance;
    return x >= left && x < left + kPanelWidth
        && y >= top && y < top + kPanelHeight;
}
