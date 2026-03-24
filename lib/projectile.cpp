#include "projectile.hpp"
#include "raylib.h"
#include "raymath.h"

#include "shader_loader.hpp"
#include <cmath>

projectile::projectile()
    : object(false)
    , position_{ 0.0f, 0.0f, 0.0f }
    , velocity_{ 0.0f, 0.0f, 10.0f }
    , color_r_(255)
    , color_g_(100)
    , color_b_(0)
    , color_alpha_(255)
    , radius_(0.15f)
    , intensity_(1.0f)
    , active_(true)
    , lifetime_(2.0f)
    , age_(0.0f)
    , trail_enabled_(true)
    , trail_length_(1.5f)
    , trail_color_r_(255)
    , trail_color_g_(50)
    , trail_color_b_(0)
    , trail_color_alpha_(200)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_position(-1)
    , loc_velocity(-1)
    , loc_age(-1)
    , loc_lifetime(-1)
{
}

projectile::projectile(const Vector3& position, const Vector3& velocity, int r, int g, int b, int alpha)
    : object(false)
    , position_(position)
    , velocity_(velocity)
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
    , radius_(0.15f)
    , intensity_(1.0f)
    , active_(true)
    , lifetime_(2.0f)
    , age_(0.0f)
    , trail_enabled_(true)
    , trail_length_(1.5f)
    , trail_color_r_(255)
    , trail_color_g_(50)
    , trail_color_b_(0)
    , trail_color_alpha_(200)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_position(-1)
    , loc_velocity(-1)
    , loc_age(-1)
    , loc_lifetime(-1)
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

    auto load_res = try_load_shader(vs_path, fs_path);
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
    loc_position = GetShaderLocation(shader_, "projPosition");
    loc_velocity = GetShaderLocation(shader_, "projVelocity");
    loc_age = GetShaderLocation(shader_, "projAge");
    loc_lifetime = GetShaderLocation(shader_, "projLifetime");
}

void projectile::unload_shader() {
    if (shader_loaded_) {
        UnloadShader(shader_);
        shader_ = { 0 };
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
    if (!active_) return;

    // Update position based on velocity
    position_ = Vector3Add(position_, Vector3Scale(velocity_, dt));

    // Update age
    age_ += dt;

    // Deactivate if lifetime exceeded
    if (age_ >= lifetime_) {
        active_ = false;
    }
}

void projectile::draw() const {
    if (!active_) return;

    // Lazy load shader on first draw
    if (!shader_loaded_) {
        projectile* mutable_this = const_cast<projectile*>(this);
        mutable_this->load_shader("projectile.vs", "projectile.fs");
    }

    if (shader_loaded_) {
        draw_with_shader_internal();
    } else {
        TraceLog(LOG_FATAL, "projectile: Failed to load shader");
    }
}

void projectile::draw_with_shader() const {
    draw_with_shader_internal();
}

void projectile::draw_with_shader_internal() const {
    const int base_alpha = calculate_age_alpha();
    if (base_alpha <= 0) return;

    // Draw trail if enabled
    if (trail_enabled_) {
        draw_trail();
    }

    // Begin shader drawing mode
    BeginShaderMode(shader_);

    // Set shader uniforms
    float color_vec4[4] = {
        color_r_ / 255.0f,
        color_g_ / 255.0f,
        color_b_ / 255.0f,
        base_alpha / 255.0f
    };
    SetShaderValue(shader_, loc_color, color_vec4, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader_, loc_intensity, &intensity_, SHADER_UNIFORM_FLOAT);

    float glow = 8.0f;
    SetShaderValue(shader_, loc_glow_radius, &glow, SHADER_UNIFORM_FLOAT);

    float position_vec3[3] = { position_.x, position_.y, position_.z };
    SetShaderValue(shader_, loc_position, position_vec3, SHADER_UNIFORM_VEC3);

    float velocity_vec3[3] = { velocity_.x, velocity_.y, velocity_.z };
    SetShaderValue(shader_, loc_velocity, velocity_vec3, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader_, loc_age, &age_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, loc_lifetime, &lifetime_, SHADER_UNIFORM_FLOAT);

    // Draw the projectile core as a small sphere (hot tip only)
    // The shader creates the glow effect around it
    const float core_radius = radius_ * 0.3f;
    DrawSphere(position_, core_radius, WHITE);

    EndShaderMode();
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

    // Add bright core at projectile base (where trail meets projectile)
    const float core_radius = radius_ * 0.5f;
    const int core_alpha = static_cast<int>(color_alpha_ * 0.6f * (calculate_age_alpha() / 255.0f));
    DrawSphere(position_, core_radius,
        Color { static_cast<unsigned char>(color_r_),
                static_cast<unsigned char>(color_g_),
                static_cast<unsigned char>(color_b_),
                static_cast<unsigned char>(core_alpha) });
}
