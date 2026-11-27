#pragma once

class camera_3d;
class ship;

// Screen-space locked-target marker: four arcs around the hull, plus an edge
// chevron when the target is outside the viewport.
class target_marker {
public:
    void draw(const camera_3d& camera, int width, int height, const ship& target,
              float distance_km, float progress, bool locked) const;
};
