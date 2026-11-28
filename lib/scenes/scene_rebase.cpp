#include "scenes/scene_rebase.hpp"

#include <raymath.h>

scene_rebase scene_rebase::between(
    const Vector3& source_player, const Vector3& destination_player) {
    return scene_rebase {
        Vector3Subtract(destination_player, source_player)
    };
}

Vector3 scene_rebase::apply(const Vector3& position) const {
    return Vector3Add(position, translation);
}
