#include "ship/ship.hpp"

#include <raylib.h>
#include <raymath.h>

#include <cmath>

#include "combat/weapon_mount.hpp"
#include "object/model_object.hpp"
#include "object/ship_object.hpp"
#include "rendering/mesh_renderer.hpp"

ship::ship(mesh_renderer& renderer, const ship_class_def& cls, const Vector3& position,
           float yaw_deg)
    : renderer_(&renderer)
    , class_(&cls)
    , position_(position)
    , yaw_deg_(yaw_deg)
    , health_(cls.durability) {
    switch (cls.visual) {
    case hull_visual::procedural_titan:
        hull_ = std::make_unique<ship_object>(
            renderer, hull_class::titan, position, yaw_deg);
        break;
    case hull_visual::glb: {
        auto model = std::make_unique<model_object>(
            renderer, cls.model_path, cls.model_target_length, position, yaw_deg);
        if (!model->loaded()) {
            TraceLog(LOG_WARNING, "ship %s: model %s failed to load", cls.id, cls.model_path);
        }
        hull_ = std::move(model);
        break;
    }
    }

    mounts_.resize(class_->hardpoints.size());
}

ship::~ship() = default;

Matrix ship::model_matrix() const {
    return MatrixMultiply(
        MatrixRotateY(yaw_deg_ * DEG2RAD),
        MatrixTranslate(position_.x, position_.y, position_.z));
}

void ship::set_position(const Vector3& position) {
    position_ = position;
    if (hull_) {
        hull_->set_pose(position_, yaw_deg_);
    }
}

void ship::translate(const Vector3& delta) {
    set_position(Vector3Add(position_, delta));
}

void ship::set_yaw_deg(float yaw_deg) {
    while (yaw_deg > 180.0f) {
        yaw_deg -= 360.0f;
    }
    while (yaw_deg <= -180.0f) {
        yaw_deg += 360.0f;
    }

    yaw_deg_ = yaw_deg;
    if (hull_) {
        hull_->set_pose(position_, yaw_deg_);
    }
}

bool ship::align_yaw_to(float target_yaw_deg, float max_turn_deg_per_sec, float dt) {
    float delta = target_yaw_deg - yaw_deg_;
    delta = fmodf(delta + 540.0f, 360.0f) - 180.0f;

    const float step = max_turn_deg_per_sec * dt;
    if (fabsf(delta) <= step) {
        set_yaw_deg(target_yaw_deg);
        return true;
    }

    set_yaw_deg(yaw_deg_ + copysignf(step, delta));
    return false;
}

Vector3 ship::aim_point() const {
    return Vector3Add(position_, Vector3 { 0.0f, class_->length * 0.03f, 0.0f });
}

void ship::update(float dt) {
    if (hull_) {
        hull_->update(dt);
    }

    if (destroyed()) {
        clear_targets();
    }

    const Matrix model = model_matrix();
    for (auto& mount : mounts_) {
        if (mount) {
            mount->update(dt, model);
        }
    }
}

void ship::draw(const camera_3d& cam, int window_height) const {
    if (hull_) {
        hull_->draw(cam, window_height);
    }

    for (const auto& mount : mounts_) {
        if (mount) {
            mount->draw(*renderer_, cam, window_height);
        }
    }
}

bool ship::blocks_line(const Vector3& from, const Vector3& to) const {
    return hull_ ? hull_->blocks_line(from, to) : false;
}

void ship::aim_at(const Vector3& world_point) {
    target_ = world_point;
    target_ship_ = nullptr;
    has_target_ = true;

    for (auto& mount : mounts_) {
        if (mount) {
            mount->set_target(world_point);
        }
    }
}

void ship::aim_at(ship& target) {
    target_ = target.aim_point();
    target_ship_ = &target;
    has_target_ = true;

    for (auto& mount : mounts_) {
        if (mount) {
            mount->set_target(target);
        }
    }
}

void ship::clear_targets() {
    has_target_ = false;
    target_ship_ = nullptr;

    for (auto& mount : mounts_) {
        if (mount) {
            mount->clear_target();
        }
    }
}

bool ship::install_weapon(int slot_index, const weapon_def& def) {
    if (slot_index < 0 || slot_index >= static_cast<int>(mounts_.size())) {
        return false;
    }

    const hardpoint_def& slot = class_->hardpoints[slot_index];
    if (!weapon_fits(slot.size, def.size)) {
        TraceLog(LOG_WARNING, "ship %s: %s does not fit hardpoint %d",
                 class_->id, def.id, slot_index);
        return false;
    }

    mounts_[slot_index] = std::make_unique<weapon_mount>(def, slot);
    if (target_ship_) {
        mounts_[slot_index]->set_target(*target_ship_);
    } else if (has_target_) {
        mounts_[slot_index]->set_target(target_);
    }
    return true;
}

void ship::remove_weapon(int slot_index) {
    if (slot_index >= 0 && slot_index < static_cast<int>(mounts_.size())) {
        mounts_[slot_index].reset();
    }
}

const weapon_mount* ship::mount_at(int slot_index) const {
    if (slot_index < 0 || slot_index >= static_cast<int>(mounts_.size())) {
        return nullptr;
    }
    return mounts_[slot_index].get();
}

void ship::take_damage(float amount) {
    float remaining = fmaxf(amount, 0.0f);

    const float shield_absorbed = fminf(health_.shield, remaining);
    health_.shield -= shield_absorbed;
    remaining -= shield_absorbed;

    const float armor_absorbed = fminf(health_.armor, remaining);
    health_.armor -= armor_absorbed;
    remaining -= armor_absorbed;

    const float structure_absorbed = fminf(health_.structure, remaining);
    health_.structure -= structure_absorbed;
}
