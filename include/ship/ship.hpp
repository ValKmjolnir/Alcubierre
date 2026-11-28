#pragma once

#include <memory>
#include <vector>

#include "combat/weapon_def.hpp"
#include "object/object.hpp"
#include "ship/ship_class.hpp"

class game_window;
class weapon_mount;

// Runtime hull: a ship class definition, current durability and the weapons
// actually fitted to its hardpoints.
class ship: public object {
public:
    ship(game_window& win, const ship_class_def& cls, const Vector3& position,
         float yaw_deg = 0.0f);
    ~ship() override;

    ship(const ship&) = delete;
    ship& operator=(const ship&) = delete;

    void update(float dt) override;
    void draw(const camera_3d& cam, int window_height) const override;
    bool blocks_line(const Vector3& from, const Vector3& to) const override;

    const ship_class_def& ship_class() const { return *class_; }
    Vector3 position() const { return position_; }
    float yaw_deg() const { return yaw_deg_; }
    Vector3 aim_point() const;
    Matrix model_matrix() const;

    void set_position(const Vector3& position);
    void set_yaw_deg(float yaw_deg);
    bool align_yaw_to(float target_yaw_deg, float max_turn_deg_per_sec, float dt);

    void aim_at(const Vector3& world_point);
    void aim_at(ship& target);
    void clear_targets();

    bool install_weapon(int slot_index, const weapon_def& def);
    void remove_weapon(int slot_index);
    int hardpoint_count() const { return static_cast<int>(class_->hardpoints.size()); }
    const weapon_mount* mount_at(int slot_index) const;

    const ship_durability& health() const { return health_; }
    const ship_durability& max_health() const { return class_->durability; }
    void take_damage(float amount);
    bool destroyed() const { return health_.structure <= 0.0f; }

private:
    game_window* window_;
    const ship_class_def* class_;
    Vector3 position_;
    float yaw_deg_;
    ship_durability health_;
    std::unique_ptr<object> hull_;
    std::vector<std::unique_ptr<weapon_mount>> mounts_;
    Vector3 target_ { 0.0f, 0.0f, 0.0f };
    bool has_target_ = false;
    ship* target_ship_ = nullptr;
};
