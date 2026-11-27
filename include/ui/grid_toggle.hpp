#pragma once

#include <raylib.h>

// 64x64 Grid on/off tile that sits immediately left of the hull status bar.
class grid_toggle {
private:
    Rectangle button_rect(const Rectangle& status_bounds) const;

public:
    bool contains(const Rectangle& status_bounds, int x, int y) const;
    void draw(const Rectangle& status_bounds) const;
};
