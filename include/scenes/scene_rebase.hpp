#pragma once

#include <raylib.h>

struct scene_rebase {
    Vector3 translation {};

    static scene_rebase between(
        const Vector3& source_player, const Vector3& destination_player);
    Vector3 apply(const Vector3& position) const;
};
