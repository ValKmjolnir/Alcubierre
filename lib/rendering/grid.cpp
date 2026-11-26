#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "rendering/grid.hpp"
#include "utils/material_manager.hpp"
#include "utils/shader_manager.hpp"

grid::grid(float spacing, float radius, Color color) :
    spacing_(spacing), radius_(radius), color_(color) {
    auto res = shader_manager::instance().load("grid.vs", "grid.fs");
    shader_ = res.shader;
    loaded_ = res.success;
    if (!loaded_) {
        TraceLog(LOG_WARNING, "Grid shader failed to load");
        return;
    }

    loc_radius_ = GetShaderLocation(shader_, "gridRadius");
    loc_spacing_ = GetShaderLocation(shader_, "gridSpacing");
    loc_color_ = GetShaderLocation(shader_, "gridColor");

    mesh_ = GenMeshPlane(radius_ * 2.0f, radius_ * 2.0f, 1, 1);
    material_ = LoadMaterialDefault();
    material_.shader = shader_;
}

grid::~grid() {
    if (!loaded_) {
        return;
    }

    UnloadMesh(mesh_);
    safe_unload_material(material_);
}

void grid::draw(const Vector3& center) const {
    if (!loaded_) {
        return;
    }

    const float color[4] = {
        color_.r / 255.0f,
        color_.g / 255.0f,
        color_.b / 255.0f,
        color_.a / 255.0f
    };
    SetShaderValue(shader_, loc_radius_, &radius_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_spacing_, &spacing_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_color_, color, SHADER_UNIFORM_VEC4);

    const Matrix transform = MatrixTranslate(center.x, center.y, center.z);

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    BeginBlendMode(BLEND_ALPHA);
    DrawMesh(mesh_, material_, transform);
    EndBlendMode();
    rlEnableDepthMask();
    rlEnableBackfaceCulling();
}
