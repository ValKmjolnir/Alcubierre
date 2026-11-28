#pragma once

#include <raylib.h>

struct lit_surface {
    Color albedo { 255, 255, 255, 255 };
    float spec_strength = 0.0f;
    float shininess = 24.0f;
    Texture2D texture {};
};

class mesh_renderer {
public:
    virtual ~mesh_renderer() = default;
    virtual void draw_lit_mesh(const Mesh& mesh, const Matrix& transform,
                               const lit_surface& surface) = 0;
};
