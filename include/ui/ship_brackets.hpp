#pragma once

#include <raylib.h>

#include <vector>

class camera_3d;
class ship;

class ship_brackets {
public:
    void draw(const camera_3d& camera, int width, int height,
              const std::vector<ship*>& ships, const ship* target) const;
    ship* pick(const camera_3d& camera, int width, int height,
               const std::vector<ship*>& ships, const ship* target,
               Vector2 point) const;
};
