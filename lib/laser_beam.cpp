#include "laser_beam.hpp"
#include "raylib.h"
#include "raymath.h"

#include "shader_loader.hpp"
#include <cmath>

laser_beam::laser_beam()
    : object(true)
    , start_{ 0.0f, 0.0f, 0.0f }
    , end_{ 0.0f, 0.0f, 10.0f }
    , color_r_(255)
    , color_g_(0)
    , color_b_(0)
    , color_alpha_(255)
    , width_(0.05f)
    , intensity_(1.0f)
    , active_(true)
    , firing_(false)
    , pulse_enabled_(false)
    , pulse_speed_(10.0f)
    , pulse_phase_(0.0f)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_line_start(-1)
    , loc_line_end(-1)
{
}

laser_beam::laser_beam(const Vector3& start, const Vector3& end, int r, int g, int b, int alpha)
    : object(true)
    , start_(start)
    , end_(end)
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
    , width_(0.05f)
    , intensity_(1.0f)
    , active_(true)
    , firing_(false)
    , pulse_enabled_(false)
    , pulse_speed_(10.0f)
    , pulse_phase_(0.0f)
    , shader_{ 0 }
    , shader_loaded_(false)
    , loc_mvp(-1)
    , loc_color(-1)
    , loc_intensity(-1)
    , loc_glow_radius(-1)
    , loc_line_start(-1)
    , loc_line_end(-1)
{
}

laser_beam::~laser_beam() {
    unload_shader();
}

Vector3 laser_beam::start() const {
    return start_;
}

void laser_beam::set_start(const Vector3& start) {
    start_ = start;
}

Vector3 laser_beam::end() const {
    return end_;
}

void laser_beam::set_end(const Vector3& end) {
    end_ = end;
}

void laser_beam::set_positions(const Vector3& start, const Vector3& end) {
    start_ = start;
    end_ = end;
}

int laser_beam::color_r() const {
    return color_r_;
}

int laser_beam::color_g() const {
    return color_g_;
}

int laser_beam::color_b() const {
    return color_b_;
}

int laser_beam::color_alpha() const {
    return color_alpha_;
}

void laser_beam::set_color(int r, int g, int b, int alpha) {
    color_r_ = r;
    color_g_ = g;
    color_b_ = b;
    color_alpha_ = alpha;
}

float laser_beam::width() const {
    return width_;
}

void laser_beam::set_width(float width) {
    width_ = width;
}

float laser_beam::intensity() const {
    return intensity_;
}

void laser_beam::set_intensity(float intensity) {
    intensity_ = intensity;
}

bool laser_beam::is_active() const {
    return active_;
}

void laser_beam::set_active(bool active) {
    active_ = active;
}

bool laser_beam::is_firing() const {
    return firing_;
}

void laser_beam::set_firing(bool firing) {
    firing_ = firing;
}

bool laser_beam::is_pulse_enabled() const {
    return pulse_enabled_;
}

void laser_beam::set_pulse_enabled(bool enabled) {
    pulse_enabled_ = enabled;
}

float laser_beam::pulse_speed() const {
    return pulse_speed_;
}

void laser_beam::set_pulse_speed(float speed) {
    pulse_speed_ = speed;
}

void laser_beam::load_shader(const char* vs_path, const char* fs_path) {
    if (shader_loaded_) {
        unload_shader();
    }
    
    auto load_res = try_load_shader(vs_path, fs_path);
    shader_ = load_res.shader;
    shader_loaded_ = load_res.success;
    if (!shader_loaded_) {
        TraceLog(LOG_FATAL, "laser_beam: Failed to load shader");
        return;
    }

    // Get uniform locations
    loc_mvp = GetShaderLocation(shader_, "mvp");
    loc_color = GetShaderLocation(shader_, "color");
    loc_intensity = GetShaderLocation(shader_, "intensity");
    loc_glow_radius = GetShaderLocation(shader_, "glowRadius");
    loc_line_start = GetShaderLocation(shader_, "lineStart");
    loc_line_end = GetShaderLocation(shader_, "lineEnd");
}

void laser_beam::unload_shader() {
    if (shader_loaded_) {
        UnloadShader(shader_);
        shader_ = { 0 };
        shader_loaded_ = false;
    }
}

bool laser_beam::is_shader_loaded() const {
    return shader_loaded_;
}

int laser_beam::calculate_pulse_alpha() const {
    if (!pulse_enabled_) {
        return color_alpha_;
    }

    const float pulse = sinf(pulse_phase_);
    const float pulse_factor = 0.5f + 0.5f * (pulse < 0.0f? -1.0f : pulse);
    return static_cast<int>(color_alpha_ * pulse_factor);
}

void laser_beam::update(float dt) {
    if (!active_) return;

    pulse_phase_ += pulse_speed_ * dt;
}

void laser_beam::draw() const {
    if (!active_ || !firing_) return;

    // Lazy load shader on first draw
    if (!shader_loaded_) {
        laser_beam* mutable_this = const_cast<laser_beam*>(this);
        mutable_this->load_shader("laser_beam.vs", "laser_beam.fs");
    }

    if (shader_loaded_) {
        draw_with_shader_internal();
    } else {
        TraceLog(LOG_FATAL, "laser_beam: Failed to load shader");
    }
}

void laser_beam::draw_with_shader_internal() const {
    const int base_alpha = pulse_enabled_ ? calculate_pulse_alpha() : color_alpha_;
    if (base_alpha <= 0) {
        return;
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

    float glow = 12.0f;  // Wider glow
    SetShaderValue(shader_, loc_glow_radius, &glow, SHADER_UNIFORM_FLOAT);

    float start_vec3[3] = { start_.x, start_.y, start_.z };
    SetShaderValue(shader_, loc_line_start, start_vec3, SHADER_UNIFORM_VEC3);

    float end_vec3[3] = { end_.x, end_.y, end_.z };
    SetShaderValue(shader_, loc_line_end, end_vec3, SHADER_UNIFORM_VEC3);

    // Draw the laser beam as a cylinder
    const float radius = width_ * 0.5f;
    const Color color = {
        static_cast<unsigned char>(color_r_),
        static_cast<unsigned char>(color_g_),
        static_cast<unsigned char>(color_b_),
        static_cast<unsigned char>(base_alpha)
    };
    DrawCylinderEx(start_, end_, radius, radius, 8, color);

    EndShaderMode();
}
