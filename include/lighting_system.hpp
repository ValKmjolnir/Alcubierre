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

public:
    static lighting_system& instance();

    // Light management — caller is responsible for lifetime
    void add(light* light);
    void remove(light* light);
    void clear();

    // Pass light data to a shader
    // Returns the number of active lights passed
    int apply_to_shader(Shader shader) const;

    // Query
    int active_light_count() const;
    const std::vector<light*>& get_all_lights() const { return lights_; }
};
