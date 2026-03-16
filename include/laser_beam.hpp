#pragma once

#include "raylib.h"
#include "raymath.h"

class laser_beam {
public:
    laser_beam();
    laser_beam(const Vector3& start, const Vector3& end, int r = 255, int g = 0, int b = 0, int alpha = 255);
    ~laser_beam();

    // Beam properties
    Vector3 start() const;
    void set_start(const Vector3& start);

    Vector3 end() const;
    void set_end(const Vector3& end);

    void set_positions(const Vector3& start, const Vector3& end);

    int color_r() const;
    int color_g() const;
    int color_b() const;
    int color_alpha() const;
    void set_color(int r, int g, int b, int alpha = 255);

    float width() const;
    void set_width(float width);

    float intensity() const;
    void set_intensity(float intensity);

    // Beam state
    bool is_active() const;
    void set_active(bool active);

    void set_firing(bool firing);
    bool is_firing() const;

    // Animation
    void set_jitter_enabled(bool enabled);
    bool is_jitter_enabled() const;
    void set_jitter_amount(float amount);
    float jitter_amount() const;

    void set_pulse_enabled(bool enabled);
    bool is_pulse_enabled() const;
    void set_pulse_speed(float speed);
    float pulse_speed() const;

    // Update and draw
    void update(float dt);
    void draw() const;

private:
    Vector3 start_;
    Vector3 end_;
    int color_r_;
    int color_g_;
    int color_b_;
    int color_alpha_;
    float width_;
    float intensity_;
    bool active_;
    bool firing_;
    bool jitter_enabled_;
    float jitter_amount_;
    Vector3 jitter_offset_;
    float jitter_time_;
    bool pulse_enabled_;
    float pulse_speed_;
    float pulse_phase_;

    Vector3 calculate_jitter(float dt) const;
    int calculate_pulse_alpha() const;
    void draw_beam_layer(const Vector3& start, const Vector3& end, float layer_width, int alpha) const;
};
