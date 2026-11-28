#include "rendering/warp_tunnel.hpp"

#include <raymath.h>
#include <rlgl.h>

#include <cmath>

#include "utils/shader_manager.hpp"

namespace {

constexpr int kSegments = 64;
constexpr float kRadius = 400.0f;
constexpr float kNearZ = -500.0f;
constexpr float kFarZ = 30000.0f;

Mesh build_tunnel_mesh() {
    Mesh mesh = {};
    const int vertex_count = (kSegments + 1) * 2;
    const int index_count = kSegments * 6;

    mesh.vertexCount = vertex_count;
    mesh.triangleCount = kSegments * 2;
    mesh.vertices = static_cast<float*>(
        MemAlloc(sizeof(float) * 3 * vertex_count));
    mesh.texcoords = static_cast<float*>(
        MemAlloc(sizeof(float) * 2 * vertex_count));
    mesh.indices = static_cast<unsigned short*>(
        MemAlloc(sizeof(unsigned short) * index_count));

    for (int i = 0; i <= kSegments; ++i) {
        const float angle = 2.0f * PI * float(i) / float(kSegments);
        const float x = cosf(angle) * kRadius;
        const float y = sinf(angle) * kRadius;

        for (int ring = 0; ring < 2; ++ring) {
            const int vi = i * 2 + ring;
            mesh.vertices[vi * 3 + 0] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = ring == 0 ? kNearZ : kFarZ;
            mesh.texcoords[vi * 2 + 0] = float(i) / float(kSegments);
            mesh.texcoords[vi * 2 + 1] = float(ring);
        }
    }

    int index = 0;
    for (int i = 0; i < kSegments; ++i) {
        const unsigned short a = static_cast<unsigned short>(i * 2);
        const unsigned short b = static_cast<unsigned short>((i + 1) * 2);
        const unsigned short c = static_cast<unsigned short>(i * 2 + 1);
        const unsigned short d = static_cast<unsigned short>((i + 1) * 2 + 1);
        mesh.indices[index++] = a;
        mesh.indices[index++] = b;
        mesh.indices[index++] = c;
        mesh.indices[index++] = b;
        mesh.indices[index++] = d;
        mesh.indices[index++] = c;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

} // namespace

warp_tunnel::~warp_tunnel() {
    unload();
}

void warp_tunnel::load() {
    if (loaded_) {
        return;
    }

    mesh_ = build_tunnel_mesh();

    auto tunnel_shader = shader_manager::instance().load(
        "warp_tunnel.vs", "warp_tunnel.fs");
    shader_ = tunnel_shader.shader;
    loaded_ = tunnel_shader.success;

    if (!loaded_) {
        UnloadMesh(mesh_);
        mesh_ = {};
        return;
    }

    material_ = {};
    material_.shader = shader_;
    material_.maps = maps_;
    loc_amount_ = GetShaderLocation(shader_, "amount");
    loc_time_ = GetShaderLocation(shader_, "tunnelTime");
}

void warp_tunnel::unload() {
    if (loaded_) {
        UnloadMesh(mesh_);
    }
    mesh_ = {};
    material_ = {};
    for (auto& map : maps_) {
        map = {};
    }
    shader_ = {};
    loaded_ = false;
    loc_amount_ = -1;
    loc_time_ = -1;
}

void warp_tunnel::draw(const Vector3& origin, const Vector3& forward,
                       float amount, float time) const {
    if (!loaded_ || amount <= 0.001f) {
        return;
    }

    const Vector3 axis = Vector3Normalize(forward + Vector3 { 0.0001f, 0.0f, 0.0f });
    const Quaternion rotation = QuaternionFromVector3ToVector3(
        Vector3 { 0.0f, 0.0f, 1.0f }, axis);
    const Matrix transform = MatrixMultiply(
        QuaternionToMatrix(rotation),
        MatrixTranslate(origin.x, origin.y, origin.z));

    SetShaderValue(shader_, loc_amount_, &amount, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_time_, &time, SHADER_UNIFORM_FLOAT);

    BeginBlendMode(BLEND_ADDITIVE);
    rlDisableDepthTest();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    DrawMesh(mesh_, material_, transform);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
    rlEnableDepthTest();
    EndBlendMode();
}
