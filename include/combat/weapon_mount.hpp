#pragma once

#include <raylib.h>

#include <memory>
#include <vector>

#include "camera.hpp"
#include "combat/weapon_def.hpp"
#include "ship/ship_class.hpp"

class mesh_renderer;
class laser_beam;
class projectile;
class ship;

// Runtime instance of one fitted weapon: owns its turret meshes, its aim
// solution and the beam/projectile effects it spawns.
class weapon_mount {
public:
    weapon_mount(const weapon_def& def, const hardpoint_def& slot);
    ~weapon_mount();

    weapon_mount(const weapon_mount&) = delete;
    weapon_mount& operator=(const weapon_mount&) = delete;

    const weapon_def& def() const { return *def_; }
    const hardpoint_def& slot() const { return *slot_; }

    void set_target(const Vector3& world_point);
    void set_target(ship& target_ship);
    void clear_target();

    void update(float dt, const Matrix& ship_model);
    void draw(mesh_renderer& renderer, const camera_3d& cam, int window_height) const;

private:
    Vector3 fire_direction() const;
    Vector3 muzzle_position(int barrel) const;

    const weapon_def* def_;
    const hardpoint_def* slot_;

    Vector3 target_ { 0.0f, 0.0f, 0.0f };
    bool has_target_ = false;
    ship* target_ship_ = nullptr;
    float cooldown_ = 0.0f;
    float beam_damage_timer_ = 0.0f;

    Vector3 impact_point_ { 0.0f, 0.0f, 0.0f };
    float impact_time_ = 1.0f;

    Matrix base_transform_ {};
    Matrix gun_transform_ {};

    Mesh base_mesh_ {};
    Mesh housing_mesh_ {};
    Mesh barrel_mesh_ {};
    bool meshes_ready_ = false;
    std::vector<Vector3> barrel_offsets_;

    std::vector<std::unique_ptr<laser_beam>> beams_;
    std::vector<std::unique_ptr<projectile>> shots_;
};
