#include "combat/weapon_mount.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <cmath>

#include "object/laser_beam.hpp"
#include "object/projectile.hpp"
#include "ship/ship.hpp"
#include "rendering/mesh_renderer.hpp"

namespace {

constexpr float kBarrelSpacing = 0.34f;
constexpr float kBarrelAxisY = 0.30f;
constexpr float kBarrelStartZ = 0.35f;
constexpr float kImpactDuration = 0.16f;

float clampf(float value, float lo, float hi) {
    return fminf(fmaxf(value, lo), hi);
}

Vector3 dir_transform(const Vector3& d, const Matrix& m) {
    return Vector3 {
        d.x * m.m0 + d.y * m.m4 + d.z * m.m8,
        d.x * m.m1 + d.y * m.m5 + d.z * m.m9,
        d.x * m.m2 + d.y * m.m6 + d.z * m.m10,
    };
}

Vector3 dir_transform_inverse(const Vector3& d, const Matrix& m) {
    return Vector3 {
        d.x * m.m0 + d.y * m.m1 + d.z * m.m2,
        d.x * m.m4 + d.y * m.m5 + d.z * m.m6,
        d.x * m.m8 + d.y * m.m9 + d.z * m.m10,
    };
}

Vector3 any_perpendicular(const Vector3& v) {
    const Vector3 helper = (fabsf(v.y) > 0.9f) ? Vector3 { 1.0f, 0.0f, 0.0f }
                                               : Vector3 { 0.0f, 1.0f, 0.0f };
    Vector3 p = Vector3CrossProduct(helper, v);
    if (Vector3Length(p) < 1e-4f) {
        p = Vector3 { 1.0f, 0.0f, 0.0f };
    }
    return Vector3Normalize(p);
}

Matrix basis(const Vector3& x, const Vector3& y, const Vector3& z) {
    Matrix m = { 0 };
    m.m0 = x.x;
    m.m1 = x.y;
    m.m2 = x.z;
    m.m4 = y.x;
    m.m5 = y.y;
    m.m6 = y.z;
    m.m8 = z.x;
    m.m9 = z.y;
    m.m10 = z.z;
    m.m15 = 1.0f;
    return m;
}

} // namespace

weapon_mount::weapon_mount(const weapon_def& def, const hardpoint_def& slot)
    : def_(&def)
    , slot_(&slot) {
    base_mesh_ = GenMeshCylinder(1.0f, 0.18f, 16);
    housing_mesh_ = GenMeshCylinder(0.78f, 0.55f, 12);
    meshes_ready_ = true;

    for (int i = 0; i < def_->barrel_count; i++) {
        const float x = (static_cast<float>(i) - (def_->barrel_count - 1) * 0.5f) * kBarrelSpacing;
        barrel_offsets_.push_back(Vector3 {
            x, kBarrelAxisY, -(kBarrelStartZ + def_->barrel_length * 0.5f) });
    }

    if (def_->barrel_count > 0) {
        barrel_mesh_ = GenMeshCylinder(def_->barrel_radius, def_->barrel_length, 8);
    }

    if (def_->delivery == weapon_delivery::beam) {
        for (int i = 0; i < def_->barrel_count; i++) {
            auto beam = std::make_unique<laser_beam>();
            beam->set_color(def_->color.r, def_->color.g, def_->color.b, 255);
            beam->set_width(def_->beam_width);
            beam->set_pulse_enabled(def_->pulse_speed > 0.0f);
            beam->set_pulse_speed(def_->pulse_speed);
            beam->set_firing(false);
            beams_.push_back(std::move(beam));
        }
    } else if (def_->delivery == weapon_delivery::projectile) {
        for (int i = 0; i < def_->barrel_count; i++) {
            auto shot = std::make_unique<projectile>();
            shot->set_color(def_->color.r, def_->color.g, def_->color.b, 255);
            shot->set_radius(def_->projectile_radius);
            shot->set_lifetime(def_->projectile_lifetime);
            shot->set_trail_enabled(true);
            shot->set_trail_length(fmaxf(1.0f, def_->projectile_speed * 0.02f));
            shot->set_trail_color(def_->color.r, def_->color.g, def_->color.b, 180);
            shot->set_active(false);
            shots_.push_back(std::move(shot));
        }
    }
}

weapon_mount::~weapon_mount() {
    if (!meshes_ready_) {
        return;
    }
    UnloadMesh(base_mesh_);
    UnloadMesh(housing_mesh_);
    if (barrel_mesh_.vertexCount > 0) {
        UnloadMesh(barrel_mesh_);
    }
}

void weapon_mount::set_target(const Vector3& world_point) {
    target_ = world_point;
    target_ship_ = nullptr;
    has_target_ = true;
}

void weapon_mount::set_target(ship& target_ship) {
    target_ = target_ship.aim_point();
    target_ship_ = &target_ship;
    has_target_ = true;
}

void weapon_mount::clear_target() {
    has_target_ = false;
    target_ship_ = nullptr;
}

Vector3 weapon_mount::muzzle_position(int barrel) const {
    const Vector3 center = barrel_offsets_[barrel];
    const Vector3 local = Vector3Add(center, Vector3 { 0.0f, 0.0f, -def_->barrel_length * 0.5f });
    return Vector3Transform(local, gun_transform_);
}

Vector3 weapon_mount::fire_direction() const {
    return Vector3Normalize(dir_transform(Vector3 { 0.0f, 0.0f, -1.0f }, gun_transform_));
}

void weapon_mount::update(float dt, const Matrix& ship_model) {
    const float scale = def_->turret_scale;

    const Vector3 rest = Vector3Normalize(slot_->forward);
    const Vector3 mount_up = Vector3Normalize(slot_->up);

    Vector3 up_ortho = Vector3Subtract(
        mount_up, Vector3Scale(rest, Vector3DotProduct(mount_up, rest)));
    if (Vector3Length(up_ortho) < 1e-4f) {
        up_ortho = Vector3 { 0.0f, 1.0f, 0.0f };
    }
    up_ortho = Vector3Normalize(up_ortho);
    const Vector3 rest_right = Vector3Normalize(Vector3CrossProduct(rest, up_ortho));

    bool in_arc = true;
    Vector3 local_aim = rest;
    if (has_target_) {
        const Vector3 mount_world = Vector3Transform(slot_->position, ship_model);
        const Vector3 world_dir = Vector3Normalize(Vector3Subtract(target_, mount_world));
        const Vector3 local_dir = Vector3Normalize(dir_transform_inverse(world_dir, ship_model));

        const float yaw_raw = atan2f(Vector3DotProduct(local_dir, rest_right),
                                     Vector3DotProduct(local_dir, rest));
        const float pitch_raw = asinf(
            clampf(Vector3DotProduct(local_dir, up_ortho), -1.0f, 1.0f));
        const float yaw_limit = slot_->yaw_arc_deg * DEG2RAD;
        const float pitch_min = slot_->pitch_min_deg * DEG2RAD;
        const float pitch_max = slot_->pitch_max_deg * DEG2RAD;

        in_arc = fabsf(yaw_raw) <= yaw_limit
              && pitch_raw >= pitch_min && pitch_raw <= pitch_max;

        const float yaw = clampf(yaw_raw, -yaw_limit, yaw_limit);
        const float pitch = clampf(pitch_raw, pitch_min, pitch_max);

        local_aim = Vector3Add(
            Vector3Add(Vector3Scale(rest, cosf(pitch) * cosf(yaw)),
                       Vector3Scale(rest_right, cosf(pitch) * sinf(yaw))),
            Vector3Scale(up_ortho, sinf(pitch)));
        local_aim = Vector3Normalize(local_aim);
    }

    Vector3 gun_up = Vector3Subtract(
        up_ortho, Vector3Scale(local_aim, Vector3DotProduct(up_ortho, local_aim)));
    if (Vector3Length(gun_up) < 1e-4f) {
        gun_up = any_perpendicular(local_aim);
    }
    gun_up = Vector3Normalize(gun_up);
    const Vector3 gun_right = Vector3Normalize(Vector3CrossProduct(local_aim, gun_up));

    const Vector3 slot_pos = slot_->position;
    const Matrix to_ship = MatrixTranslate(slot_pos.x, slot_pos.y, slot_pos.z);

    const Vector3 base_x = any_perpendicular(mount_up);
    const Vector3 base_z = Vector3CrossProduct(base_x, mount_up);
    base_transform_ = MatrixScale(scale, scale, scale);
    base_transform_ = MatrixMultiply(
        base_transform_, MatrixTranslate(0.0f, -0.09f * scale, 0.0f));
    base_transform_ = MatrixMultiply(base_transform_, basis(base_x, mount_up, base_z));
    base_transform_ = MatrixMultiply(base_transform_, to_ship);
    base_transform_ = MatrixMultiply(base_transform_, ship_model);

    gun_transform_ = MatrixScale(scale, scale, scale);
    gun_transform_ = MatrixMultiply(
        gun_transform_, MatrixTranslate(0.0f, 0.10f * scale, 0.0f));
    gun_transform_ = MatrixMultiply(
        gun_transform_, basis(gun_right, gun_up, Vector3Scale(local_aim, -1.0f)));
    gun_transform_ = MatrixMultiply(gun_transform_, to_ship);
    gun_transform_ = MatrixMultiply(gun_transform_, ship_model);

    const Vector3 fire_dir = fire_direction();

    impact_time_ += dt;

    if (def_->delivery == weapon_delivery::beam) {
        const Vector3 mount_world = Vector3Transform(slot_->position, ship_model);
        const bool target_in_range = has_target_ && in_arc
            && Vector3Distance(mount_world, target_) <= def_->range;

        for (size_t i = 0; i < beams_.size(); i++) {
            auto& beam = *beams_[i];
            beam.update(dt);

            if (!target_in_range) {
                beam.set_firing(false);
                continue;
            }

            const Vector3 start = muzzle_position(static_cast<int>(i));
            if (Vector3Distance(start, target_) > def_->range) {
                beam.set_firing(false);
                continue;
            }

            const float reach = fmaxf(
                Vector3DotProduct(Vector3Subtract(target_, start), fire_dir), 0.0f);
            beam.set_positions(start, Vector3Add(start, Vector3Scale(fire_dir, reach)));
            beam.set_firing(true);
        }

        if (target_in_range && target_ship_) {
            beam_damage_timer_ -= dt;
            if (beam_damage_timer_ <= 0.0f) {
                target_ship_->take_damage(def_->damage);
                beam_damage_timer_ += def_->cooldown;
            }
        } else {
            beam_damage_timer_ = 0.0f;
        }
    } else if (def_->delivery == weapon_delivery::projectile) {
        cooldown_ -= dt;
        for (auto& shot : shots_) {
            const Vector3 previous = shot->position();
            shot->update(dt);

            if (!shot->is_active() || !target_ship_) {
                continue;
            }

            if (target_ship_->blocks_line(previous, shot->position())) {
                target_ship_->take_damage(def_->damage);
                shot->set_active(false);
                impact_point_ = previous;
                impact_time_ = 0.0f;
            }
        }

        if (has_target_ && in_arc && cooldown_ <= 0.0f) {
            const Vector3 mount_world = Vector3Transform(slot_->position, ship_model);
            if (Vector3Distance(mount_world, target_) <= def_->range) {
                for (size_t i = 0; i < shots_.size(); i++) {
                    auto& shot = *shots_[i];
                    if (shot.is_active()) {
                        continue;
                    }
                    shot.set_position(muzzle_position(static_cast<int>(i)));
                    shot.set_velocity(Vector3Scale(fire_dir, def_->projectile_speed));
                    shot.set_age(0.0f);
                    shot.set_active(true);
                }
                cooldown_ = def_->cooldown;
            }
        }
    }
}

void weapon_mount::draw(mesh_renderer& renderer, const camera_3d& cam, int window_height) const {
    lit_surface base_surface;
    base_surface.albedo = Color { 48, 52, 60, 255 };
    base_surface.spec_strength = 0.45f;
    base_surface.shininess = 14.0f;
    renderer.draw_lit_mesh(base_mesh_, base_transform_, base_surface);

    lit_surface gun_surface;
    gun_surface.albedo = Color { 74, 79, 88, 255 };
    gun_surface.spec_strength = 0.7f;
    gun_surface.shininess = 18.0f;
    renderer.draw_lit_mesh(housing_mesh_, gun_transform_, gun_surface);

    if (def_->barrel_count > 0) {
        lit_surface barrel_surface;
        barrel_surface.albedo = Color { 96, 101, 112, 255 };
        barrel_surface.spec_strength = 0.9f;
        barrel_surface.shininess = 20.0f;

        for (const auto& offset : barrel_offsets_) {
            Matrix local = MatrixTranslate(0.0f, -def_->barrel_length * 0.5f, 0.0f);
            local = MatrixMultiply(local, MatrixRotateX(-90.0f * DEG2RAD));
            local = MatrixMultiply(local, MatrixTranslate(offset.x, offset.y, offset.z));
            renderer.draw_lit_mesh(barrel_mesh_, MatrixMultiply(local, gun_transform_), barrel_surface);
        }
    }

    for (const auto& beam : beams_) {
        beam->draw(cam, window_height);
    }
    for (const auto& shot : shots_) {
        shot->draw(cam, window_height);
    }

    if (impact_time_ < kImpactDuration) {
        const float t = clampf(impact_time_ / kImpactDuration, 0.0f, 1.0f);
        const float radius = fmaxf(def_->projectile_radius * 6.0f, 0.25f) * (1.0f - 0.5f * t);
        const Color color {
            static_cast<unsigned char>(fminf(def_->color.r + 60.0f, 255.0f)),
            static_cast<unsigned char>(fminf(def_->color.g + 60.0f, 255.0f)),
            static_cast<unsigned char>(fminf(def_->color.b + 60.0f, 255.0f)),
            static_cast<unsigned char>(255.0f * (1.0f - t))
        };

        rlDisableDepthMask();
        BeginBlendMode(BLEND_ADDITIVE);
        DrawSphereEx(impact_point_, radius, 6, 8, color);
        EndBlendMode();
        rlEnableDepthMask();
    }
}
