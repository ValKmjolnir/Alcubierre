#pragma once

#include <raylib.h>

#include <vector>

#include "combat/weapon_def.hpp"

enum class ship_size {
    frigate,
    cruiser,
    battleship,
    titan
};

enum class hull_visual {
    procedural_corvette,
    procedural_titan,
    glb
};

struct ship_durability {
    float shield;
    float armor;
    float structure;
};

// Hull-local frame in world units: +Y up, -Z nose. Yaw is a half-angle around
// forward; pitch is measured toward the hardpoint's up axis.
struct hardpoint_def {
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    weapon_size size;
    float yaw_arc_deg;
    float pitch_min_deg;
    float pitch_max_deg;
};

struct ship_class_def {
    const char* id;
    const char* name;
    ship_size size;
    float length;
    hull_visual visual;
    const char* model_path;
    float model_target_length;
    ship_durability durability;
    std::vector<hardpoint_def> hardpoints;
};

const ship_class_def* ship_class_catalog();
int ship_class_catalog_size();
const ship_class_def* find_ship_class(const char* id);
