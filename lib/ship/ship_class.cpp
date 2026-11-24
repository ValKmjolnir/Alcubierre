#include "ship/ship_class.hpp"

namespace {

const Vector3 kForward { 0.0f, 0.0f, -1.0f };
const Vector3 kUp { 0.0f, 1.0f, 0.0f };
const Vector3 kDown { 0.0f, -1.0f, 0.0f };

std::vector<ship_class_def> build_catalog() {
    return {
        {
            "corvette_glb", "Corvette", ship_size::frigate, 15.0f,
            hull_visual::glb, "assets/models/ship_py.glb", 15.0f,
            { 600.0f, 800.0f, 900.0f },
            {
                // Socket rings authored in tool/build_ship.py, converted to
                // hull-local world units (already centred and scaled to 15).
                { { 0.0f, 1.12f, -4.36f }, kForward, kUp, weapon_size::small, 140.0f, -10.0f, 80.0f },
                { { 0.0f, 1.48f, -1.84f }, kForward, kUp, weapon_size::small, 140.0f, -10.0f, 80.0f },
                { { -0.81f, 1.03f, -2.56f }, kForward, kUp, weapon_size::small, 140.0f, -10.0f, 80.0f },
                { { 0.81f, 1.03f, -2.56f }, kForward, kUp, weapon_size::small, 140.0f, -10.0f, 80.0f },
                { { 0.0f, 1.48f, 1.93f }, kForward, kUp, weapon_size::small, 140.0f, -10.0f, 80.0f },
                { { -0.54f, -0.54f, -4.81f }, kForward, kDown, weapon_size::small, 140.0f, 0.0f, 70.0f },
                { { 0.54f, -0.54f, -4.81f }, kForward, kDown, weapon_size::small, 140.0f, 0.0f, 70.0f },
            },
        },
        {
            "titan_blockout", "Titan", ship_size::titan, 1000.0f,
            hull_visual::procedural_titan, nullptr, 0.0f,
            { 180000.0f, 240000.0f, 300000.0f },
            {
                // Spinal mount sits on the lance housing; the two dorsal
                // barbettes step up the spine, so their mount planes differ.
                { { 0.0f, 82.0f, -300.0f }, kForward, kUp, weapon_size::capital, 6.0f, -5.0f, 12.0f },
                { { 0.0f, 80.0f, -190.0f }, kForward, kUp, weapon_size::capital, 150.0f, -10.0f, 80.0f },
                { { 0.0f, 88.0f, -60.0f }, kForward, kUp, weapon_size::capital, 150.0f, -10.0f, 80.0f },
                { { -42.0f, -57.0f, -150.0f }, kForward, kDown, weapon_size::large, 140.0f, 0.0f, 70.0f },
                { { 42.0f, -57.0f, 110.0f }, kForward, kDown, weapon_size::large, 140.0f, 0.0f, 70.0f },
                { { -60.0f, 65.0f, -230.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { 60.0f, 65.0f, -230.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { -60.0f, 65.0f, 280.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { 60.0f, 65.0f, 280.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
            },
        },
    };
}

const std::vector<ship_class_def>& catalog() {
    static const std::vector<ship_class_def> classes = build_catalog();
    return classes;
}

} // namespace

const ship_class_def* ship_class_catalog() {
    return catalog().data();
}

int ship_class_catalog_size() {
    return static_cast<int>(catalog().size());
}

const ship_class_def* find_ship_class(const char* id) {
    for (const auto& cls : catalog()) {
        if (TextIsEqual(cls.id, id)) {
            return &cls;
        }
    }
    return nullptr;
}
