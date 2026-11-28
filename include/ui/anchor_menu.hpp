#pragma once

#include <raylib.h>

enum class anchor_action {
    none,
    align_to,
    warp_to
};

class anchor_menu {
public:
    void open(Vector2 mouse, int width, int height);
    void close();
    bool visible() const;
    anchor_action update();
    void draw() const;

private:
    Rectangle bounds_ {};
    bool visible_ = false;
    int pressed_ = -1;
};
