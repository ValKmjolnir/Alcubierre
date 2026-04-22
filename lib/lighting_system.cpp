#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cstdio>

#include "lighting_system.hpp"

lighting_system& lighting_system::instance() {
    static lighting_system inst;
    return inst;
}

void lighting_system::add(std::shared_ptr<light> light) {
    if (!light) {
        return;
    }
    if (std::find(lights_.begin(), lights_.end(), light) != lights_.end()) {
        return;
    }

    lights_.push_back(light);
}

void lighting_system::remove(std::shared_ptr<light> light) {
    lights_.erase(
        std::remove(lights_.begin(), lights_.end(), light),
        lights_.end()
    );
}

void lighting_system::clear() {
    lights_.clear();
}

int lighting_system::apply_to_shader(Shader shader) const {
    int active_count = 0;

    for (const auto& l : lights_) {
        if (!l || !l->is_active()) continue;

        // Build uniform name prefix for this light index
        char pos_name[64];
        char color_name[64];
        char intensity_name[64];
        char type_name[64];

        snprintf(pos_name, sizeof(pos_name), "lights[%d].position", active_count);
        snprintf(color_name, sizeof(color_name), "lights[%d].color", active_count);
        snprintf(intensity_name, sizeof(intensity_name), "lights[%d].intensity", active_count);
        snprintf(type_name, sizeof(type_name), "lights[%d].type", active_count);

        int loc_pos = GetShaderLocation(shader, pos_name);
        int loc_color = GetShaderLocation(shader, color_name);
        int loc_intensity = GetShaderLocation(shader, intensity_name);
        int loc_type = GetShaderLocation(shader, type_name);

        // Pass position
        if (loc_pos >= 0) {
            Vector3 pos = l->position();
            float pos_data[3] = { pos.x, pos.y, pos.z };
            SetShaderValue(shader, loc_pos, pos_data, SHADER_UNIFORM_VEC3);
        }

        // Pass color
        if (loc_color >= 0) {
            Vector3 col = l->color();
            float col_data[3] = { col.x, col.y, col.z };
            SetShaderValue(shader, loc_color, col_data, SHADER_UNIFORM_VEC3);
        }

        // Pass intensity
        if (loc_intensity >= 0) {
            float intensity = l->intensity();
            SetShaderValue(shader, loc_intensity, &intensity, SHADER_UNIFORM_FLOAT);
        }

        // Pass type
        if (loc_type >= 0) {
            float type_val = static_cast<float>(l->type());
            SetShaderValue(shader, loc_type, &type_val, SHADER_UNIFORM_FLOAT);
        }

        active_count++;
    }

    // Also pass the total light count
    int loc_count = GetShaderLocation(shader, "light_count");
    if (loc_count >= 0) {
        SetShaderValue(shader, loc_count, &active_count, SHADER_UNIFORM_INT);
    }

    return active_count;
}

int lighting_system::active_light_count() const {
    int count = 0;
    for (const auto& l : lights_) {
        if (l && l->is_active()) count++;
    }
    return count;
}
