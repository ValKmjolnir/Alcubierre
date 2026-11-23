#pragma once

#include <raylib.h>

enum class weapon_category {
    offensive,
    defensive,
    support
};

enum class weapon_size {
    small,
    medium,
    large,
    capital
};

enum class weapon_delivery {
    beam,
    projectile,
    utility
};

// A weapon is defined once and fitted to any hardpoint it is size-compatible
// with. Turret geometry is authored in canonical units (base ring radius = 1)
// and scaled by turret_scale, so capital weapons and corvette guns share one
// mesh layout.
struct weapon_def {
    const char* id;
    const char* name;
    weapon_category category;
    weapon_size size;
    weapon_delivery delivery;

    float damage;
    float range;
    float cooldown;

    float turret_scale;
    int barrel_count;
    float barrel_length;
    float barrel_radius;

    Color color;
    float beam_width;
    float pulse_speed;
    float projectile_speed;
    float projectile_radius;
    float projectile_lifetime;
};

int weapon_size_rank(weapon_size size);
bool weapon_fits(weapon_size slot, weapon_size weapon);

const weapon_def* weapon_catalog();
int weapon_catalog_size();
const weapon_def* find_weapon(const char* id);
