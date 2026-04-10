#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

#include "skybox.hpp"
#include "utils/shader_loader.hpp"

skybox::skybox() : is_loaded_(false), seed_(42.0f), seed_location_(-1) {
    TraceLog(LOG_DEBUG, "Creating skybox...");
    init_mesh();
    load_shader();
    TraceLog(LOG_DEBUG, "Skybox created, is_loaded_ = %d", is_loaded_);
}

skybox::~skybox() {
    TraceLog(LOG_DEBUG, "Skybox destructor called, is_loaded_ = %d", is_loaded_);
    if (is_loaded_) {
        TraceLog(LOG_DEBUG, "Unloading skybox mesh and shader");
        UnloadMesh(mesh_);
        UnloadShader(shader_);
    }
}

void skybox::init_mesh() {
    // Create a large cube for the skybox
    mesh_ = GenMeshCube(500.0f, 500.0f, 500.0f);

    // Upload mesh to GPU
    UploadMesh(&mesh_, false);
}

void skybox::load_shader() {
    auto load_res = try_load_shader("skybox.vs", "skybox.fs");
    shader_ = load_res.shader;
    if (!load_res.success) {
        TraceLog(LOG_FATAL, "skybox: Failed to load shader");
        return;
    }

    TraceLog(LOG_INFO, "After LoadShader - shader.id = %d", shader_.id);

    // Check shader compilation status
    if (shader_.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load skybox shader (shader.id == 0)");
        TraceLog(LOG_ERROR, "Check console for shader compilation errors above");
        is_loaded_ = false;
        return;
    }

    TraceLog(LOG_INFO, "Skybox shader loaded, shader.id = %d", shader_.id);

    // Check all uniform locations
    int mvp_loc = GetShaderLocation(shader_, "mvp");
    TraceLog(LOG_INFO, "Uniform 'mvp' location = %d", mvp_loc);

    seed_location_ = GetShaderLocation(shader_, "seed");
    TraceLog(LOG_INFO, "Uniform 'seed' location = %d", seed_location_);

    if (mvp_loc == -1) {
        TraceLog(LOG_WARNING, "Skybox shader: 'mvp' uniform not found");
        TraceLog(LOG_WARNING, "Check console for shader compilation errors above");
    } else {
        TraceLog(LOG_INFO, "Skybox shader loaded successfully, mvp_loc = %d", mvp_loc);
    }

    // Set default seed value
    if (seed_location_ != -1) {
        SetShaderValue(shader_, seed_location_, &seed_, 1);
    }

    is_loaded_ = true;
}

void skybox::draw(const Camera3D& camera) {
    // Get the view matrix without translation (skybox follows camera)
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

    // Remove translation from view matrix (skybox stays at infinity)
    view.m12 = 0.0f;
    view.m13 = 0.0f;
    view.m14 = 0.0f;

    // Get projection matrix using MatrixPerspective
    const double fov_rad = camera.fovy * DEG2RAD;
    const double aspect = 16.0 / 9.0;  // Approximate aspect ratio
    const double near_plane = 0.01;
    const double far_plane = 1000.0;
    Matrix proj = MatrixPerspective(fov_rad, aspect, near_plane, far_plane);

    // Calculate MVP matrix
    Matrix mvp = MatrixMultiply(view, proj);

    if (!is_loaded_) {
        // Fallback: draw with default material (white cube)
        TraceLog(LOG_DEBUG, "Skybox::draw: using fallback default material");
        DrawMesh(mesh_, LoadMaterialDefault(), MatrixIdentity());
        return;
    }

    // Set shader uniforms
    int mvp_loc = GetShaderLocation(shader_, "mvp");
    if (mvp_loc == -1) {
        TraceLog(LOG_WARNING, "Skybox::draw: 'mvp' uniform location not found");
        // Fallback: draw with default material
        DrawMesh(mesh_, LoadMaterialDefault(), MatrixIdentity());
        return;
    }

    SetShaderValueMatrix(shader_, mvp_loc, mvp);

    // Update seed uniform
    if (seed_location_ != -1) {
        SetShaderValue(shader_, seed_location_, &seed_, 1);
    }

    TraceLog(LOG_DEBUG, "Skybox::draw: MVP uniform set, drawing skybox");

    // Disable depth write and depth test for skybox
    // Skybox should always be drawn in the background
    rlDisableDepthMask();
    rlDisableDepthTest();

    // Disable backface culling so we can see the cube from inside
    rlDisableBackfaceCulling();

    // Create a material to use the skybox shader
    Material material = LoadMaterialDefault();
    material.shader = shader_;

    DrawMesh(mesh_, material, MatrixIdentity());

    // Re-enable backface culling, depth test and depth write
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
    rlEnableDepthMask();
}

bool skybox::is_loaded() const {
    return is_loaded_;
}

void skybox::set_seed(float seed) {
    seed_ = seed;
    if (is_loaded_ && seed_location_ != -1) {
        SetShaderValue(shader_, seed_location_, &seed_, 1);
    }
}
