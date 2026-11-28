#include "utils/screen_projection.hpp"

#include <raymath.h>

#include "camera.hpp"

namespace screen_projection {

bool project(const camera_3d& camera, int width, int height,
             const Vector3& world, Vector2* screen, float margin,
             float min_forward_dot) {
    const Vector3 forward = camera.forward();
    if (Vector3DotProduct(Vector3Subtract(world, camera.position()), forward)
        <= min_forward_dot) {
        return false;
    }

    const Vector2 projected = GetWorldToScreenEx(
        world, camera.get_camera(), width, height);
    if (projected.x < -margin || projected.x > float(width) + margin
        || projected.y < -margin || projected.y > float(height) + margin) {
        return false;
    }

    *screen = projected;
    return true;
}

} // namespace screen_projection
