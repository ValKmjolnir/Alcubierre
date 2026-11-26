#include <raylib.h>

#include <cstdio>

#include "ui/ship_status.hpp"
#include "ship/ship_class.hpp"
#include "window.hpp"

namespace {

constexpr int kPanelWidth = 460;
constexpr int kPanelHeight = 64;
constexpr int kMargin = 24;
constexpr int kPadding = 8;
constexpr int kLabelWidth = 84;
constexpr int kValueWidth = 96;
constexpr int kRowHeight = 14;
constexpr int kRowGap = 6;
constexpr int kRowTop = 6;

constexpr float kGhostHoldSeconds = 0.35f;
constexpr float kGhostDecayPerSecond = 0.9f;

Color layer_color(int index) {
    switch (index) {
    case 0: return Color { 0x58, 0xb4, 0xff, 255 };
    case 1: return Color { 0xff, 0xb4, 0x58, 255 };
    default: return Color { 0xff, 0x6a, 0x5a, 255 };
    }
}

const char* layer_label(int index) {
    switch (index) {
    case 0: return "SHIELD";
    case 1: return "ARMOR";
    default: return "STRUCTURE";
    }
}

float layer_fraction(float current, float maximum) {
    if (maximum <= 0.0f) {
        return 0.0f;
    }
    const float fraction = current / maximum;
    if (fraction < 0.0f) {
        return 0.0f;
    }
    if (fraction > 1.0f) {
        return 1.0f;
    }
    return fraction;
}

}

ship_status::ship_status(game_window& window) : window_(window) {}

void ship_status::update_layer(int index, float fraction, float dt, float hold_seconds) {
    if (fraction < ghost_[index]) {
        if (hold_[index] > 0.0f) {
            hold_[index] -= dt;
        } else {
            ghost_[index] = fmaxf(fraction, ghost_[index] - dt * kGhostDecayPerSecond);
        }
    } else {
        ghost_[index] = fraction;
        hold_[index] = hold_seconds;
    }
}

void ship_status::update(float dt, const ship_durability& current, const ship_durability& maximum) {
    const float shield = layer_fraction(current.shield, maximum.shield);
    const float armor = layer_fraction(current.armor, maximum.armor);
    const float structure = layer_fraction(current.structure, maximum.structure);

    fraction_[0] = shield;
    fraction_[1] = armor;
    fraction_[2] = structure;

    current_value_[0] = int(current.shield);
    current_value_[1] = int(current.armor);
    current_value_[2] = int(current.structure);
    max_value_[0] = int(maximum.shield);
    max_value_[1] = int(maximum.armor);
    max_value_[2] = int(maximum.structure);

    for (int i = 0; i < 3; i++) {
        if (!initialized_) {
            ghost_[i] = fraction_[i];
            hold_[i] = 0.0f;
        }
        update_layer(i, fraction_[i], dt, kGhostHoldSeconds);
    }
    initialized_ = true;
}

void ship_status::draw() const {
    const int x = (window_.width() - kPanelWidth) / 2;
    const int y = window_.height() - kPanelHeight - kMargin;

    DrawRectangle(x, y, kPanelWidth, kPanelHeight, Color { 10, 14, 20, 205 });
    DrawRectangleLines(x, y, kPanelWidth, kPanelHeight, Color { 0x00, 0xdd, 0xff, 90 });

    const int bar_x = x + kPadding + kLabelWidth;
    const int bar_width = kPanelWidth - kPadding * 2 - kLabelWidth - kValueWidth;

    for (int i = 0; i < 3; i++) {
        const int row_y = y + kRowTop + i * (kRowHeight + kRowGap);
        const Color color = layer_color(i);
        const float fraction = fraction_[i];

        const Color label_color = (fraction > 0.0f) ? color : Color { 110, 120, 132, 255 };
        DrawText(layer_label(i), x + kPadding, row_y + 1, 12, label_color);

        DrawRectangle(bar_x, row_y, bar_width, kRowHeight, Color { 26, 32, 42, 230 });

        const int ghost_width = int(bar_width * ghost_[i]);
        if (ghost_width > 0) {
            DrawRectangle(bar_x, row_y, ghost_width, kRowHeight, Fade(color, 0.35f));
        }

        const int fill_width = int(bar_width * fraction);
        if (fill_width > 0) {
            DrawRectangle(bar_x, row_y, fill_width, kRowHeight, color);
        }

        char value[32];
        snprintf(value, sizeof(value), "%d/%d", current_value_[i], max_value_[i]);
        DrawText(value,
                 x + kPanelWidth - kPadding - MeasureText(value, 12),
                 row_y + 1,
                 12,
                 Color { 200, 210, 220, 255 });
    }
}
