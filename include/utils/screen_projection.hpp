#pragma once

#include <raylib.h>

class camera_3d;

namespace screen_projection {

// World point -> viewport coordinates. Returns false when the point is behind
// the camera or outside the viewport extended by `margin` pixels.
bool project(const camera_3d& camera, int width, int height,
             const Vector3& world, Vector2* screen, float margin = 0.0f,
             float min_forward_dot = 0.0f);

} // namespace screen_projection
