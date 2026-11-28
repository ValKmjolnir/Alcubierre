#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cmath>
#include <vector>

#include "rendering/skybox.hpp"
#include "utils/material_manager.hpp"
#include "utils/shader_manager.hpp"

namespace {

constexpr int kCubemapSize = 1024;
constexpr int kSplatSamples = 3;

struct star_layer {
    float grid_scale;
    float star_size;
    float weight;
    float glow_weight;
    float seed_mul;
};

const star_layer kStarLayers[] = {
    { 33.0f, 0.055f, 0.7f, 0.4f, 1.1f },
    { 33.0f, 0.055f, 0.35f, 0.0f, 7.9f },
    { 11.0f, 0.0275f, 0.45f, 0.0f, 2.7f },
    { 11.0f, 0.0275f, 0.6f, 0.2f, 3.4f },
    { 2.75f, 0.01375f, 0.25f, 0.0f, 4.3f },
    { 2.75f, 0.01375f, 0.85f, 0.1f, 2.1f },
};

float fractf(float x) {
    return x - floorf(x);
}

float hash3(const Vector3& p, float seed) {
    Vector3 p3 = {
        fractf(p.x * 0.1031f + seed * 0.318f),
        fractf(p.y * 0.1030f + seed * 0.318f),
        fractf(p.z * 0.0973f + seed * 0.318f)
    };

    const float d = p3.x * (p3.y + 33.33f) + p3.y * (p3.z + 33.33f) + p3.z * (p3.x + 33.33f);
    p3.x += d;
    p3.y += d;
    p3.z += d;

    return fractf((p3.x + p3.y) * p3.z);
}

Vector3 hash_offset3(const Vector3& cell, float seed) {
    return {
        hash3(cell, seed) - 0.5f,
        hash3(cell, seed * 2.0f) - 0.5f,
        hash3(cell, seed * 3.0f) - 0.5f
    };
}

float smooth_step(float edge0, float edge1, float x) {
    const float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

void cubemap_face(const Vector3& dir, int& face, float& s, float& t) {
    const float ax = fabsf(dir.x);
    const float ay = fabsf(dir.y);
    const float az = fabsf(dir.z);

    if (ax >= ay && ax >= az) {
        face = dir.x >= 0.0f ? 0 : 1;
        s = (dir.x >= 0.0f ? -dir.z : dir.z) / ax;
        t = -dir.y / ax;
    } else if (ay >= az) {
        face = dir.y >= 0.0f ? 2 : 3;
        s = dir.x / ay;
        t = (dir.y >= 0.0f ? dir.z : -dir.z) / ay;
    } else {
        face = dir.z >= 0.0f ? 4 : 5;
        s = (dir.z >= 0.0f ? dir.x : -dir.x) / az;
        t = -dir.y / az;
    }
}

Vector3 cubemap_direction(int face, float s, float t) {
    Vector3 dir;
    switch (face) {
        case 0: dir = { 1.0f, -t, -s }; break;
        case 1: dir = { -1.0f, -t, s }; break;
        case 2: dir = { s, 1.0f, t }; break;
        case 3: dir = { s, -1.0f, -t }; break;
        case 4: dir = { s, -t, 1.0f }; break;
        default: dir = { -s, -t, -1.0f }; break;
    }

    return Vector3Normalize(dir);
}

void star_color(const Vector3& dir, float seed, float& r, float& g, float& b) {
    const Vector3 cell = { floorf(dir.x * 100.0f), floorf(dir.y * 100.0f), floorf(dir.z * 100.0f) };
    const float value = hash3(cell, seed);

    if (value < 0.5f) {
        r = 1.0f; g = 1.0f; b = 1.0f;
    } else if (value < 0.65f) {
        r = 0.75f; g = 0.85f; b = 1.0f;
    } else if (value < 0.8f) {
        r = 1.0f; g = 0.95f; b = 0.8f;
    } else if (value < 0.9f) {
        r = 1.0f; g = 0.8f; b = 0.6f;
    } else {
        r = 0.9f; g = 0.7f; b = 0.5f;
    }
}

void add_texel(unsigned char* texel, float r, float g, float b) {
    texel[0] = (unsigned char)fminf(255.0f, texel[0] + r * 255.0f);
    texel[1] = (unsigned char)fminf(255.0f, texel[1] + g * 255.0f);
    texel[2] = (unsigned char)fminf(255.0f, texel[2] + b * 255.0f);
}

float layer_peak_gain(const star_layer& layer) {
    const float r_tex = asinf(layer.star_size / layer.grid_scale) * kCubemapSize * 0.5f;
    const float texel_u = 1.0f / r_tex;
    float peak = 0.0f;

    for (int sy = 0; sy < kSplatSamples; sy++) {
        for (int sx = 0; sx < kSplatSamples; sx++) {
            const float ux = (sx + 0.5f) * texel_u / kSplatSamples;
            const float uy = (sy + 0.5f) * texel_u / kSplatSamples;
            const float w = smooth_step(1.0f, 0.0f, sqrtf(ux * ux + uy * uy));

            peak += layer.weight * w;
            if (layer.glow_weight > 0.0f) {
                peak += layer.glow_weight * powf(w * w, 0.4f);
            }
        }
    }

    peak /= static_cast<float>(kSplatSamples * kSplatSamples);

    const float true_peak = layer.weight + layer.glow_weight;
    return peak > 0.0f ? true_peak / peak : 1.0f;
}

void splat_star(unsigned char* pixels, const Vector3& star, float grid_scale, float star_size,
                float weight, float glow_weight, float cr, float cg, float cb) {
    const float radius_p = grid_scale;
    const float star_len = Vector3Length(star);
    const Vector3 star_dir = Vector3Scale(star, 1.0f / star_len);

    const float cos_alpha = Clamp(
        (radius_p * radius_p + star_len * star_len - star_size * star_size) / (2.0f * radius_p * star_len),
        -1.0f, 1.0f);
    const int radius = static_cast<int>(ceilf(acosf(cos_alpha) * kCubemapSize * 0.5f)) + 1;

    int face;
    float sc, tc;
    cubemap_face(star_dir, face, sc, tc);

    const int cx = static_cast<int>((sc * 0.5f + 0.5f) * kCubemapSize);
    const int cy = static_cast<int>((tc * 0.5f + 0.5f) * kCubemapSize);
    const float step = 2.0f / kCubemapSize / kSplatSamples;

    for (int dy = -radius; dy <= radius; dy++) {
        const int y = cy + dy;
        if (y < 0 || y >= kCubemapSize) continue;

        for (int dx = -radius; dx <= radius; dx++) {
            const int x = cx + dx;
            if (x < 0 || x >= kCubemapSize) continue;

            const float s0 = 2.0f * x / kCubemapSize - 1.0f;
            const float t0 = 2.0f * y / kCubemapSize - 1.0f;

            float amount = 0.0f;
            for (int sy = 0; sy < kSplatSamples; sy++) {
                for (int sx = 0; sx < kSplatSamples; sx++) {
                    const Vector3 texel_dir = cubemap_direction(
                        face, s0 + (sx + 0.5f) * step, t0 + (sy + 0.5f) * step);
                    const float dist = sqrtf(fmaxf(
                        radius_p * radius_p + star_len * star_len -
                        2.0f * radius_p * star_len * Vector3DotProduct(texel_dir, star_dir),
                        0.0f));
                    const float w = smooth_step(star_size, 0.0f, dist);

                    amount += weight * w;
                    if (glow_weight > 0.0f) {
                        amount += glow_weight * powf(w * w, 0.4f);
                    }
                }
            }
            amount /= static_cast<float>(kSplatSamples * kSplatSamples);

            if (amount <= 0.0f) continue;

            add_texel(pixels + ((face * kCubemapSize + y) * kCubemapSize + x) * 4,
                      amount * cr, amount * cg, amount * cb);
        }
    }
}

}

skybox::skybox() :
    is_loaded_(false),
    seed_(42.0f),
    mvp_location_(-1),
    velocity_location_(-1) {
    TraceLog(LOG_DEBUG, "Creating skybox...");
    init_mesh();
    load_shader();
    default_material_ = LoadMaterialDefault();
    TraceLog(LOG_DEBUG, "Skybox created, is_loaded_ = %d", is_loaded_);
}

skybox::~skybox() {
    TraceLog(LOG_DEBUG, "Skybox destructor called, is_loaded_ = %d", is_loaded_);
    if (is_loaded_) {
        TraceLog(LOG_DEBUG, "Unloading skybox mesh and shader");
        UnloadMesh(mesh_);
        safe_unload_material(material_);
        safe_unload_material(default_material_);
    }
    unload_cubemap();
}

void skybox::init_mesh() {
    // Create a large cube for the skybox
    mesh_ = GenMeshCube(600.0f, 600.0f, 600.0f);
}

void skybox::load_shader() {
    auto load_res = shader_manager::instance().load("skybox.vs", "skybox.fs");
    shader_ = load_res.shader;
    if (!load_res.success) {
        TraceLog(LOG_FATAL, "skybox: Failed to load shader");
        return;
    }

    // Check shader compilation status
    if (shader_.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load skybox shader (shader.id == 0)");
        TraceLog(LOG_ERROR, "Check console for shader compilation errors above");
        is_loaded_ = false;
        return;
    }

    // Check all uniform locations
    mvp_location_ = GetShaderLocation(shader_, "skyMvp");
    velocity_location_ = GetShaderLocation(shader_, "velocity");

    if (mvp_location_ == -1) {
        TraceLog(LOG_WARNING, "Skybox shader: 'skyMvp' uniform not found");
        TraceLog(LOG_WARNING, "Check console for shader compilation errors above");
    }

    material_ = LoadMaterialDefault();
    material_.shader = shader_;
    shader_.locs[SHADER_LOC_MAP_CUBEMAP] = GetShaderLocation(shader_, "skybox");

    build_cubemap();

    is_loaded_ = true;
}

void skybox::build_cubemap() {
    std::vector<unsigned char> pixels(static_cast<size_t>(kCubemapSize) * kCubemapSize * 6 * 4);

    for (size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i + 0] = 0;
        pixels[i + 1] = 13;
        pixels[i + 2] = 26;
        pixels[i + 3] = 255;
    }

    for (const star_layer& layer : kStarLayers) {
        const float layer_seed = seed_ * layer.seed_mul;
        const float gain = layer_peak_gain(layer);
        const int reach = static_cast<int>(ceilf(layer.grid_scale + layer.star_size + 1.0f));

        for (int cz = -reach; cz <= reach; cz++) {
            for (int cy = -reach; cy <= reach; cy++) {
                for (int cx = -reach; cx <= reach; cx++) {
                    const Vector3 cell = {
                        static_cast<float>(cx),
                        static_cast<float>(cy),
                        static_cast<float>(cz)
                    };
                    const float cell_seed = layer_seed + 0.5f * (cell.x + cell.y + cell.z);
                    const Vector3 offset = hash_offset3(cell, cell_seed);
                    const Vector3 star = {
                        cell.x + offset.x,
                        cell.y + offset.y,
                        cell.z + offset.z
                    };

                    const float distance = Vector3Length(star);
                    if (distance < 0.0001f) continue;
                    if (fabsf(distance - layer.grid_scale) >= layer.star_size) continue;

                    const Vector3 dir = Vector3Scale(star, 1.0f / distance);
                    float r, g, b;
                    star_color(dir, seed_ * 7.0f, r, g, b);

                    splat_star(pixels.data(), star, layer.grid_scale, layer.star_size,
                               layer.weight * gain, layer.glow_weight * gain, r, g, b);
                }
            }
        }
    }

    unload_cubemap();

    cubemap_.id = rlLoadTextureCubemap(pixels.data(), kCubemapSize,
                                       RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    cubemap_.width = kCubemapSize;
    cubemap_.height = kCubemapSize;
    cubemap_.mipmaps = 1;
    cubemap_.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    material_.maps[MATERIAL_MAP_CUBEMAP].texture = cubemap_;
}

void skybox::unload_cubemap() {
    if (cubemap_.id > 0) {
        UnloadTexture(cubemap_);
        cubemap_ = { 0 };
    }
}

void skybox::draw(const Camera3D& camera, Vector3 velocity) {
    // Get the view matrix without translation (skybox follows camera)
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

    // Remove translation from view matrix (skybox stays at infinity)
    view.m12 = 0.0f;
    view.m13 = 0.0f;
    view.m14 = 0.0f;

    // Get projection matrix using MatrixPerspective
    const double fov_rad = camera.fovy * DEG2RAD;
    const double aspect = static_cast<double>(GetScreenWidth()) / static_cast<double>(GetScreenHeight());
    const double near_plane = 0.01;
    const double far_plane = 1000.0;
    Matrix proj = MatrixPerspective(fov_rad, aspect, near_plane, far_plane);

    // Calculate MVP matrix
    Matrix mvp = MatrixMultiply(view, proj);

    if (!is_loaded_) {
        // Fallback: draw with default material (white cube)
        TraceLog(LOG_DEBUG, "Skybox::draw: using fallback default material");
        DrawMesh(mesh_, default_material_, MatrixIdentity());
        return;
    }

    // Set shader uniforms
    if (mvp_location_ == -1) {
        TraceLog(LOG_WARNING, "Skybox::draw: 'mvp' uniform location not found");
        // Fallback: draw with default material
        DrawMesh(mesh_, default_material_, MatrixIdentity());
        return;
    }

    SetShaderValueMatrix(shader_, mvp_location_, mvp);

    if (velocity_location_ != -1) {
        float velocity_vec[3] = { velocity.x, velocity.y, velocity.z };
        SetShaderValue(shader_, velocity_location_, velocity_vec, SHADER_UNIFORM_VEC3);
    }

    TraceLog(LOG_DEBUG, "Skybox::draw: MVP uniform set, drawing skybox");

    // Disable depth write and depth test for skybox
    // Skybox should always be drawn in the background
    rlDisableDepthMask();
    rlDisableDepthTest();

    // Disable backface culling so we can see the cube from inside
    rlDisableBackfaceCulling();

    DrawMesh(mesh_, material_, MatrixIdentity());

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
    if (is_loaded_) {
        build_cubemap();
    }
}
