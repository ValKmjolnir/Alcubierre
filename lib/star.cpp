#include "star.hpp"
#include "lighting_system.hpp"
#include "raylib.h"
#include "raymath.h"

#include "shader_loader.hpp"
#include <cmath>

star::star()
    : point_light()
    , object(true)
    , color_r_(255)
    , color_g_(200)
    , color_b_(100)
    , color_alpha_(255)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_star_position(-1)
{
    lighting_system::instance().add_light(std::shared_ptr<star>(this, [](star*){}));
}

star::star(const Vector3& position, float radius, int r, int g, int b, int alpha)
    : point_light(position, { r / 255.0f, g / 255.0f, b / 255.0f }, 1.0f)
    , object(true)
    , radius_(radius)
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_star_position(-1)
{
    lighting_system::instance().add_light(std::shared_ptr<star>(this, [](star*){}));
}

star::~star() {
    unload_shader();
    lighting_system::instance().remove_light(std::shared_ptr<star>(this, [](star*){}));
}

Vector3 star::position() const {
    return position_;
}

void star::set_position(const Vector3& position) {
    position_ = position;
}

float star::radius() const {
    return radius_;
}

void star::set_radius(float radius) {
    radius_ = radius;
}

int star::color_r() const {
    return color_r_;
}

int star::color_g() const {
    return color_g_;
}

int star::color_b() const {
    return color_b_;
}

int star::color_alpha() const {
    return color_alpha_;
}

void star::set_color(int r, int g, int b, int alpha) {
    color_r_ = r;
    color_g_ = g;
    color_b_ = b;
    color_alpha_ = alpha;
    color_ = { r / 255.0f, g / 255.0f, b / 255.0f };
}

float star::intensity() const {
    return intensity_;
}

void star::set_intensity(float intensity) {
    intensity_ = intensity;
}

void star::load_shader(const char* vs_path, const char* fs_path) {

    auto load_res = try_load_shader(vs_path, fs_path);
    shader_ = load_res.shader;
    shader_loaded_ = load_res.success;
    if (!shader_loaded_) {
        TraceLog(LOG_WARNING, "star: Failed to load shader");
        return;
    }

    // Get uniform locations
    loc_mvp = GetShaderLocation(shader_, "mvp");
    loc_color = GetShaderLocation(shader_, "color");
    loc_intensity = GetShaderLocation(shader_, "intensity");
    loc_glow_radius = GetShaderLocation(shader_, "glowRadius");
    loc_star_position = GetShaderLocation(shader_, "starPosition");
}

void star::unload_shader() {
    if (shader_loaded_) {
        UnloadShader(shader_);
        shader_ = { 0 };
        shader_loaded_ = false;
    }
}

bool star::is_shader_loaded() const {
    return shader_loaded_;
}

void star::draw() const {
    if (!active_) return;

    // Lazy load shader on first draw
    if (!shader_loaded_) {
        star* mutable_this = const_cast<star*>(this);
        mutable_this->load_shader("star.vs", "star.fs");
    }

    if (shader_loaded_) {
        draw_with_shader_internal();
    } else {
        TraceLog(LOG_WARNING, "star: Shader not loaded, using default rendering");
        DrawSphere(position_, radius_, Color {
            static_cast<unsigned char>(color_r_),
            static_cast<unsigned char>(color_g_),
            static_cast<unsigned char>(color_b_),
            static_cast<unsigned char>(color_alpha_)
        });
    }
}

void star::draw_with_shader_internal() const {
    // Begin shader drawing mode
    BeginShaderMode(shader_);

    // Set shader uniforms
    float color_vec4[4] = {
        color_r_ / 255.0f,
        color_g_ / 255.0f,
        color_b_ / 255.0f,
        color_alpha_ / 255.0f
    };
    SetShaderValue(shader_, loc_color, color_vec4, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader_, loc_intensity, &intensity_, SHADER_UNIFORM_FLOAT);

    float glow_radius = 20.0f;
    SetShaderValue(shader_, loc_glow_radius, &glow_radius, SHADER_UNIFORM_FLOAT);

    float position_vec3[3] = { position_.x, position_.y, position_.z };
    SetShaderValue(shader_, loc_star_position, position_vec3, SHADER_UNIFORM_VEC3);

    // Draw the star as a sphere
    const Color color = {
        static_cast<unsigned char>(color_r_),
        static_cast<unsigned char>(color_g_),
        static_cast<unsigned char>(color_b_),
        static_cast<unsigned char>(color_alpha_)
    };
    DrawSphere(position_, radius_, color);

    EndShaderMode();
}
