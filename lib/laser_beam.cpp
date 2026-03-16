#include "laser_beam.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

laser_beam::laser_beam()
    : start_{ 0.0f, 0.0f, 0.0f }
    , end_{ 0.0f, 0.0f, 10.0f }
    , color_r_(255)
    , color_g_(0)
    , color_b_(0)
    , color_alpha_(255)
    , width_(0.01f)
    , intensity_(1.0f)
    , active_(true)
    , firing_(false)
    , jitter_enabled_(true)
    , jitter_amount_(0.05f)
    , jitter_offset_{ 0.0f, 0.0f, 0.0f }
    , jitter_time_(0.0f)
    , pulse_enabled_(false)
    , pulse_speed_(10.0f)
    , pulse_phase_(0.0f)
{
}

laser_beam::laser_beam(const Vector3& start, const Vector3& end, int r, int g, int b, int alpha)
    : start_(start)
    , end_(end)
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
    , width_(0.01f)
    , intensity_(1.0f)
    , active_(true)
    , firing_(false)
    , jitter_enabled_(true)
    , jitter_amount_(0.05f)
    , jitter_offset_{ 0.0f, 0.0f, 0.0f }
    , jitter_time_(0.0f)
    , pulse_enabled_(false)
    , pulse_speed_(10.0f)
    , pulse_phase_(0.0f)
{
}

laser_beam::~laser_beam() = default;

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

bool laser_beam::is_jitter_enabled() const {
    return jitter_enabled_;
}

void laser_beam::set_jitter_enabled(bool enabled) {
    jitter_enabled_ = enabled;
}

float laser_beam::jitter_amount() const {
    return jitter_amount_;
}

void laser_beam::set_jitter_amount(float amount) {
    jitter_amount_ = amount;
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

Vector3 laser_beam::calculate_jitter(float dt) const {
    if (!jitter_enabled_ || !firing_) {
        return { 0.0f, 0.0f, 0.0f };
    }

    Vector3 jitter;
    jitter.x = (GetRandomValue(-1000, 1000) / 1000.0f) * jitter_amount_;
    jitter.y = (GetRandomValue(-1000, 1000) / 1000.0f) * jitter_amount_;
    jitter.z = (GetRandomValue(-1000, 1000) / 1000.0f) * jitter_amount_;
    return jitter;
}

int laser_beam::calculate_pulse_alpha() const {
    if (!pulse_enabled_) {
        return color_alpha_;
    }

    const float pulse = sinf(pulse_phase_);
    const float pulse_factor = 0.5f + 0.5f * pulse;
    return static_cast<int>(color_alpha_ * intensity_ * pulse_factor);
}

void laser_beam::draw_beam_layer(const Vector3& start, const Vector3& end, float layer_width, int alpha) const {
    if (alpha <= 0) return;

    const Color layer_color = {
        static_cast<unsigned char>(color_r_),
        static_cast<unsigned char>(color_g_),
        static_cast<unsigned char>(color_b_),
        static_cast<unsigned char>(alpha)
    };

    const float radius = layer_width * 0.5f;
    DrawCylinderEx(start, end, radius, radius, 8, layer_color);
}

void laser_beam::update(float dt) {
    if (!active_) return;

    jitter_time_ += dt;
    pulse_phase_ += pulse_speed_ * dt;

    if (jitter_enabled_ && firing_) {
        jitter_offset_ = calculate_jitter(dt);
    } else {
        jitter_offset_ = { 0.0f, 0.0f, 0.0f };
    }
}

void laser_beam::draw() const {
    if (!active_ || !firing_) return;

    Vector3 draw_start = start_;
    Vector3 draw_end = end_;

    if (firing_ && jitter_enabled_) {
        draw_end = {
            end_.x + jitter_offset_.x,
            end_.y + jitter_offset_.y,
            end_.z + jitter_offset_.z
        };
    }

    const int base_alpha = pulse_enabled_ ? calculate_pulse_alpha() : color_alpha_;

    if (base_alpha <= 0) return;

    draw_beam_layer(draw_start, draw_end, width_ * 4.0f, static_cast<int>(base_alpha * 0.3f * intensity_));
    draw_beam_layer(draw_start, draw_end, width_ * 2.5f, static_cast<int>(base_alpha * 0.5f * intensity_));
    draw_beam_layer(draw_start, draw_end, width_ * 1.5f, static_cast<int>(base_alpha * 0.8f * intensity_));
    draw_beam_layer(draw_start, draw_end, width_, base_alpha);
}
