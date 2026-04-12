#pragma once

#include "light_base.hpp"
#include "raylib.h"

#include <memory>
#include <vector>

class lighting_system {
public:
    static lighting_system& instance();

    // Light management
    void add_light(std::shared_ptr<light> light);
    void remove_light(std::shared_ptr<light> light);
    void clear_lights();

    // Pass light data to a shader
    // Returns the number of active lights passed
    int apply_to_shader(Shader shader) const;

    // Query
    int active_light_count() const;
    const std::vector<std::shared_ptr<light>>& get_all_lights() const;

private:
    lighting_system() = default;
    ~lighting_system() = default;
    lighting_system(const lighting_system&) = delete;
    lighting_system& operator=(const lighting_system&) = delete;

    std::vector<std::shared_ptr<light>> lights_;
};
