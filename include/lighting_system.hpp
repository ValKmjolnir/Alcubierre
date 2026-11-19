#pragma once

#include <raylib.h>
#include <vector>

#include "light_base.hpp"

class lighting_system {
private:
    lighting_system() = default;
    ~lighting_system() = default;
    lighting_system(const lighting_system&) = delete;
    lighting_system& operator=(const lighting_system&) = delete;

    std::vector<light*> lights_;
    Vector3 view_position_ { 0.0f, 0.0f, 0.0f };

    struct shader_locs {
        unsigned int shader_id = 0;
        int count = -1;
        int view_pos = -1;
        std::vector<int> position;
        std::vector<int> color;
        std::vector<int> intensity;
        std::vector<int> type;
        std::vector<int> direction;
        std::vector<int> ground_color;
        std::vector<int> cutoff;
        std::vector<int> inner_cutoff;
        std::vector<int> range;
    };
    mutable shader_locs locs_;

public:
    static lighting_system& instance();

    // Light management — caller is responsible for lifetime
    void add(light* light);
    void remove(light* light);
    void clear();

    // Pass light data to a shader
    // Returns the number of active lights passed
    int apply_to_shader(Shader shader) const;

    // Camera position, needed by view-dependent terms such as specular
    void set_view_position(const Vector3& position) { view_position_ = position; }

    // Query
    int active_light_count() const;
    const std::vector<light*>& get_all_lights() const { return lights_; }
};
