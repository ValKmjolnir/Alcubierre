#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cstdio>

#include "lighting_system.hpp"

lighting_system& lighting_system::instance() {
    static lighting_system inst;
    return inst;
}

void lighting_system::add(light* light) {
    if (!light) {
        return;
    }
    if (std::find(lights_.begin(), lights_.end(), light) != lights_.end()) {
        return;
    }

    lights_.push_back(light);
}

void lighting_system::remove(light* light) {
    lights_.erase(
        std::remove(lights_.begin(), lights_.end(), light),
        lights_.end()
    );
}

void lighting_system::clear() {
    lights_.clear();
}

int lighting_system::apply_to_shader(Shader shader) const {
    if (locs_.shader_id != shader.id) {
        locs_.shader_id = shader.id;
        locs_.count = GetShaderLocation(shader, "light_count");
        locs_.position.clear();
        locs_.color.clear();
        locs_.intensity.clear();
        locs_.type.clear();
    }

    int active_count = 0;

    for (const auto& l : lights_) {
        if (!l || !l->is_active()) {
            continue;
        }

        while (locs_.position.size() <= static_cast<std::size_t>(active_count)) {
            char name[64];
            int index = static_cast<int>(locs_.position.size());

            snprintf(name, sizeof(name), "lights[%d].position", index);
            locs_.position.push_back(GetShaderLocation(shader, name));
            snprintf(name, sizeof(name), "lights[%d].color", index);
            locs_.color.push_back(GetShaderLocation(shader, name));
            snprintf(name, sizeof(name), "lights[%d].intensity", index);
            locs_.intensity.push_back(GetShaderLocation(shader, name));
            snprintf(name, sizeof(name), "lights[%d].type", index);
            locs_.type.push_back(GetShaderLocation(shader, name));
        }

        int loc_pos = locs_.position[active_count];
        int loc_color = locs_.color[active_count];
        int loc_intensity = locs_.intensity[active_count];
        int loc_type = locs_.type[active_count];

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

    if (locs_.count >= 0) {
        SetShaderValue(shader, locs_.count, &active_count, SHADER_UNIFORM_INT);
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
