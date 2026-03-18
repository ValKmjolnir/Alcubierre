#pragma once

#include "raylib.h"
#include <string>

class skybox {
public:
    skybox();
    ~skybox();

    // Draw the skybox
    void draw(const Camera3D& camera);

    // Check if skybox is loaded
    bool is_loaded() const;

private:
    void init_mesh();
    void load_shader();

    Mesh mesh_;
    Shader shader_;
    bool is_loaded_;
};
