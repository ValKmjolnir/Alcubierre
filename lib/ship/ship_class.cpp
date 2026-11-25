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
            hull_visual::glb, "assets/models/titan_py.glb", 1000.0f,
            { 180000.0f, 240000.0f, 300000.0f },
            {
                // Sockets authored in tool/build_titan.py, already converted to
                // hull-local world units (centred, scaled to 1000).  The model
                // is re-centred on its bounds, so re-copy this table whenever
                // the hull's bounding box changes.
                { { 0.0f, -2.0f, -299.9f }, kForward, kUp, weapon_size::capital, 6.0f, -5.0f, 12.0f },
                { { 0.0f, 29.0f, -199.9f }, kForward, kUp, weapon_size::capital, 150.0f, -10.0f, 80.0f },
                { { 0.0f, 35.0f, 180.1f }, kForward, kUp, weapon_size::capital, 150.0f, -10.0f, 80.0f },
                { { -26.0f, -40.0f, 90.1f }, kForward, kDown, weapon_size::large, 140.0f, 0.0f, 70.0f },
                { { 26.0f, -39.0f, 230.0f }, kForward, kDown, weapon_size::large, 140.0f, 0.0f, 70.0f },
                { { -33.0f, 14.0f, -159.9f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { 33.0f, 14.0f, -159.9f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { -35.0f, 18.0f, 250.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
                { { 35.0f, 18.0f, 250.0f }, kForward, kUp, weapon_size::small, 120.0f, -10.0f, 75.0f },
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
