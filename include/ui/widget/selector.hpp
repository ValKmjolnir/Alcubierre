#pragma once

#include <raylib.h>

#include "ui/widget/widget.hpp"

class game_window;

struct resolution_option {
    int width;
    int height;
    const char* label;
};

class selector: public widget {
private:
    int x;
    int y;
    int width;
    int height;

    Color color;

    const resolution_option* options = nullptr;
    int option_count = 0;
    int pressed_side = 0;
    game_window* window = nullptr;

private:
    Rectangle arrow_left() const;
    Rectangle arrow_right() const;
    Rectangle middle() const;
    int current_index() const;
    void draw_text_centered(const Rectangle& target, const char* text, int fontsize, Color color);

public:
    selector(int x,
             int y,
             int width,
             int height,
             Color color,
             const resolution_option* options,
             int option_count,
             game_window* window) :
        x(x), y(y),
        width(width), height(height),
        color(color),
        options(options), option_count(option_count),
        window(window) {}

    void draw() override;
    void move(int dx, int dy) override;
    bool is_hovered(int mouse_x, int mouse_y) const override;
    void mouse_press_call_back() override;
    void mouse_release_call_back() override;
};
