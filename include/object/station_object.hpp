#pragma once

#include <raylib.h>
#include <raymath.h>

#include "object/object.hpp"

class mesh_renderer;
class camera_3d;

class station_object: public object {
public:
    station_object(mesh_renderer& renderer, const Vector3& position,
                   float yaw_deg = 0.0f);
    ~station_object() override;

    station_object(const station_object&) = delete;
    station_object& operator=(const station_object&) = delete;

    void draw(const camera_3d& camera, int window_height) const override;
    bool blocks_line(const Vector3& from, const Vector3& to) const override;

private:
    mesh_renderer* renderer_;
    Vector3 position_;
    float yaw_deg_;

    const Mesh* ring_;
    const Mesh* core_;
    const Mesh* arm_x_;
    const Mesh* arm_z_;
    const Mesh* module_;
};
