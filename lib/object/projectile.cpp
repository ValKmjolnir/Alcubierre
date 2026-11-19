#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cmath>

#include "object/projectile.hpp"
#include "utils/material_manager.hpp"
#include "utils/shader_manager.hpp"

namespace {

Mesh make_glow_quad() {
    Mesh mesh = { 0 };
    mesh.vertexCount = 4;
    mesh.triangleCount = 2;
    mesh.vertices = static_cast<float*>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.texcoords = static_cast<float*>(MemAlloc(mesh.vertexCount * 2 * sizeof(float)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(6 * sizeof(unsigned short)));

    const float pos[4][3] = { { -1.0f, -1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f },
                              { 1.0f, 1.0f, 0.0f }, { -1.0f, 1.0f, 0.0f } };
    const float uv[4][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    const unsigned short idx[6] = { 0, 1, 2, 0, 2, 3 };

    for (int i = 0; i < 4; i++) {
        mesh.vertices[i * 3 + 0] = pos[i][0];
        mesh.vertices[i * 3 + 1] = pos[i][1];
        mesh.vertices[i * 3 + 2] = pos[i][2];
        mesh.normals[i * 3 + 0] = 0.0f;
        mesh.normals[i * 3 + 1] = 0.0f;
        mesh.normals[i * 3 + 2] = 1.0f;
        mesh.texcoords[i * 2 + 0] = uv[i][0];
        mesh.texcoords[i * 2 + 1] = uv[i][1];
    }
    for (int i = 0; i < 6; i++) {
        mesh.indices[i] = idx[i];
    }

    UploadMesh(&mesh, false);
    return mesh;
}

} // namespace

projectile::projectile()
    : object()
    , position_{ 0.0f, 0.0f, 0.0f }
    , velocity_{ 0.0f, 0.0f, 10.0f }
    , color_r_(255)
    , color_g_(100)
    , color_b_(0)
    , color_alpha_(255)
    , radius_(0.15f)
    , intensity_(1.5f)
    , active_(true)
    , lifetime_(2.0f)
    , age_(0.0f)
    , trail_enabled_(true)
    , trail_length_(1.5f)
    , trail_color_r_(255)
    , trail_color_g_(50)
    , trail_color_b_(0)
    , trail_color_alpha_(200)
{
}

projectile::projectile(const Vector3& position, const Vector3& velocity, int r, int g, int b, int alpha)
    : object()
    , position_(position)
    , velocity_(velocity)
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
    , radius_(0.15f)
    , intensity_(1.5f)
    , active_(true)
    , lifetime_(2.0f)
    , age_(0.0f)
    , trail_enabled_(true)
    , trail_length_(1.5f)
    , trail_color_r_(255)
    , trail_color_g_(50)
    , trail_color_b_(0)
    , trail_color_alpha_(200)
{
}

projectile::~projectile() {
    unload_shader();
}

Vector3 projectile::position() const {
    return position_;
}

void projectile::set_position(const Vector3& position) {
    position_ = position;
}

Vector3 projectile::velocity() const {
    return velocity_;
}

void projectile::set_velocity(const Vector3& velocity) {
    velocity_ = velocity;
}

int projectile::color_r() const {
    return color_r_;
}

int projectile::color_g() const {
    return color_g_;
}

int projectile::color_b() const {
    return color_b_;
}

int projectile::color_alpha() const {
    return color_alpha_;
}

void projectile::set_color(int r, int g, int b, int alpha) {
    color_r_ = r;
    color_g_ = g;
    color_b_ = b;
    color_alpha_ = alpha;
}

float projectile::radius() const {
    return radius_;
}

void projectile::set_radius(float radius) {
    radius_ = radius;
}

float projectile::intensity() const {
    return intensity_;
}

void projectile::set_intensity(float intensity) {
    intensity_ = intensity;
}

bool projectile::is_active() const {
    return active_;
}

void projectile::set_active(bool active) {
    active_ = active;
}

float projectile::lifetime() const {
    return lifetime_;
}

void projectile::set_lifetime(float lifetime) {
    lifetime_ = lifetime;
}

float projectile::age() const {
    return age_;
}

void projectile::set_age(float age) {
    age_ = age;
}

bool projectile::is_trail_enabled() const {
    return trail_enabled_;
}

void projectile::set_trail_enabled(bool enabled) {
    trail_enabled_ = enabled;
}

float projectile::trail_length() const {
    return trail_length_;
}

void projectile::set_trail_color(int r, int g, int b, int alpha) {
    trail_color_r_ = r;
    trail_color_g_ = g;
    trail_color_b_ = b;
    trail_color_alpha_ = alpha;
}

void projectile::set_trail_length(float length) {
    trail_length_ = length;
}

void projectile::load_shader(const char* vs_path, const char* fs_path) {
    if (shader_loaded_) {
        unload_shader();
    }

    auto load_res = shader_manager::instance().load(vs_path, fs_path);
    shader_ = load_res.shader;
    shader_loaded_ = load_res.success;
    if (!shader_loaded_) {
        TraceLog(LOG_FATAL, "projectile: Failed to load shader");
        return;
    }

    // Get uniform locations
    loc_mvp = GetShaderLocation(shader_, "mvp");
    loc_color = GetShaderLocation(shader_, "color");
    loc_intensity = GetShaderLocation(shader_, "intensity");
    loc_glow_radius = GetShaderLocation(shader_, "glowRadius");
    loc_core_frac = GetShaderLocation(shader_, "coreFrac");
    loc_cam_right = GetShaderLocation(shader_, "camRight");
    loc_cam_up = GetShaderLocation(shader_, "camUp");
    loc_position = GetShaderLocation(shader_, "projPosition");
    loc_age = GetShaderLocation(shader_, "projAge");
    loc_lifetime = GetShaderLocation(shader_, "projLifetime");

    glow_mesh_ = make_glow_quad();
    glow_material_ = LoadMaterialDefault();
    glow_material_.shader = shader_;
    glow_ready_ = true;
}

void projectile::unload_shader() {
    if (glow_ready_) {
        safe_unload_material(glow_material_);
        UnloadMesh(glow_mesh_);
        glow_ready_ = false;
    }
    if (shader_loaded_) {
        shader_loaded_ = false;
    }
}

bool projectile::is_shader_loaded() const {
    return shader_loaded_;
}

int projectile::calculate_age_alpha() const {
    if (age_ >= lifetime_) {
        return 0;
    }

    // Fade out in the last 20% of lifetime
    const float fade_start = lifetime_ * 0.8f;
    if (age_ > fade_start) {
        const float fade_factor = 1.0f - (age_ - fade_start) / (lifetime_ - fade_start);
        return static_cast<int>(color_alpha_ * fade_factor);
    }

    return color_alpha_;
}

void projectile::update(float dt) {
    if (!active_) {
        if (auto_respawn_) {
            position_ = respawn_point_;
            active_ = true;
            age_ = 0.0f;
        }
        return;
    }

    position_ = Vector3Add(position_, Vector3Scale(velocity_, dt));
    age_ += dt;

    if (age_ >= lifetime_) {
        active_ = false;
    }
}

void projectile::set_auto_respawn(const Vector3& point) {
    respawn_point_ = point;
    auto_respawn_ = true;
}

void projectile::draw(const camera_3d& cam, int /*window_height*/) const {
    if (!active_) return;

    // Lazy load shader on first draw
    if (!shader_loaded_) {
        projectile* mutable_this = const_cast<projectile*>(this);
        mutable_this->load_shader("projectile.vs", "projectile.fs");
    }
    if (!shader_loaded_) {
        TraceLog(LOG_FATAL, "projectile: Failed to load shader");
    }

    const int base_alpha = calculate_age_alpha();
    if (base_alpha <= 0) return;

    // Draw trail if enabled
    if (trail_enabled_) {
        draw_trail();
    }

    const float hot = Clamp(intensity_ * 0.5f, 0.0f, 1.0f);
    const Color core_color = {
        static_cast<unsigned char>(color_r_ + (255 - color_r_) * hot),
        static_cast<unsigned char>(color_g_ + (255 - color_g_) * hot),
        static_cast<unsigned char>(color_b_ + (255 - color_b_) * hot),
        static_cast<unsigned char>(base_alpha)
    };
    DrawSphere(position_, radius_ * 0.45f, core_color);

    // Additive glow, bright enough to clear the bloom luma threshold
    const float core_radius = fmaxf(radius_ * 0.9f, 0.11f);
    const float glow_radius = core_radius * 3.0f;
    if (!glow_ready_) {
        return;
    }

    rlDisableDepthMask();
    BeginBlendMode(BLEND_ADDITIVE);

    float color_vec4[4] = {
        color_r_ / 255.0f,
        color_g_ / 255.0f,
        color_b_ / 255.0f,
        base_alpha / 255.0f
    };
    SetShaderValue(shader_, loc_color, color_vec4, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader_, loc_intensity, &intensity_, SHADER_UNIFORM_FLOAT);

    float glow = glow_radius;
    float core_frac = core_radius / glow_radius;
    SetShaderValue(shader_, loc_glow_radius, &glow, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_core_frac, &core_frac, SHADER_UNIFORM_FLOAT);

    const Vector3 forward = Vector3Normalize(Vector3Subtract(cam.target(), cam.position()));
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up()));
    const Vector3 up = Vector3CrossProduct(right, forward);

    float right_vec3[3] = { right.x, right.y, right.z };
    float up_vec3[3] = { up.x, up.y, up.z };
    SetShaderValue(shader_, loc_cam_right, right_vec3, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, loc_cam_up, up_vec3, SHADER_UNIFORM_VEC3);

    float position_vec3[3] = { position_.x, position_.y, position_.z };
    SetShaderValue(shader_, loc_position, position_vec3, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader_, loc_age, &age_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_lifetime, &lifetime_, SHADER_UNIFORM_FLOAT);

    DrawMesh(glow_mesh_, glow_material_, MatrixIdentity());

    EndBlendMode();
    rlEnableDepthMask();
}

void projectile::draw_trail() const {
    if (age_ < 0.01f) return;

    // Calculate trail length based on velocity and age
    const float trail_speed = Vector3Length(velocity_);
    const float trail_len = fminf(trail_length_, trail_speed * age_);

    if (trail_len < 0.01f) return;

    // Trail direction is opposite to velocity (trail goes backward)
    Vector3 trail_dir = { 0 };
    if (trail_speed > 0.001f) {
        trail_dir = Vector3Scale(velocity_, -1.0f / trail_speed);
    } else {
        trail_dir = { 0.0f, 0.0f, -1.0f };
    }

    // Trail end position (behind the projectile)
    const Vector3 trail_end = Vector3Add(position_, Vector3Scale(trail_dir, trail_len));

    // Draw trail as multiple segmented cylinders for smoother fade
    const int segments = 8;
    const float segment_length = trail_len / static_cast<float>(segments);

    for (int i = 0; i < segments; i++) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float inv_t = 1.0f - t;

        // Calculate segment positions (from projectile backward)
        // t=0: at projectile, t=1: at trail end
        const Vector3 seg_start = Vector3Add(position_, Vector3Scale(trail_dir, t * trail_len));
        const Vector3 seg_end = Vector3Add(seg_start, Vector3Scale(trail_dir, segment_length));

        // Radius tapers: large near projectile (inv_t=1), small at trail end (inv_t=0)
        const float radius_factor = powf(inv_t, 0.5f);
        const float seg_radius = radius_ * 0.4f * radius_factor;

        // Alpha fades: bright near projectile (inv_t=1), dim at trail end (inv_t=0)
        const float alpha_factor = powf(inv_t, 1.5f);
        const int seg_alpha = static_cast<int>(trail_color_alpha_ * alpha_factor * (calculate_age_alpha() / 255.0f));

        if (seg_alpha > 5) {
            DrawCylinderEx(seg_start, seg_end, seg_radius * 0.7f, seg_radius, 8,
                Color { static_cast<unsigned char>(trail_color_r_),
                        static_cast<unsigned char>(trail_color_g_),
                        static_cast<unsigned char>(trail_color_b_),
                        static_cast<unsigned char>(seg_alpha) });
        }
    }
}
