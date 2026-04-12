#pragma once

#include "raylib.h"
#include "raymath.h"

#include "object/object.hpp"

class laser_beam: public object {
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
    bool pulse_enabled_;
    float pulse_speed_;
    float pulse_phase_;

    // Shader members
    Shader shader_;
    bool shader_loaded_;
    int loc_mvp;
    int loc_color;
    int loc_intensity;
    int loc_glow_radius;
    int loc_line_start;
    int loc_line_end;

    int calculate_pulse_alpha() const;
    void draw_with_shader_internal() const;

public:
    laser_beam();
    laser_beam(const Vector3& start, const Vector3& end, int r = 255, int g = 0, int b = 0, int alpha = 255);
    ~laser_beam();

    // Beam properties
    Vector3 start() const { return start_; }
    void set_start(const Vector3& start) { start_ = start; }
    Vector3 end() const { return end_; }
    void set_end(const Vector3& end) { end_ = end; }

    void set_positions(const Vector3& start, const Vector3& end) {
        start_ = start;
        end_ = end;
    }

    int color_r() const { return color_r_; }
    int color_g() const { return color_g_; }
    int color_b() const { return color_b_; }
    int color_alpha() const { return color_alpha_; }
    void set_color(int r, int g, int b, int alpha = 255) {
        color_r_ = r;
        color_g_ = g;
        color_b_ = b;
        color_alpha_ = alpha;
    }

    float width() const { return width_; }
    void set_width(float width) { width_ = width; }
    float intensity() const { return intensity_; }
    void set_intensity(float intensity) { intensity_ = intensity; }

    // Beam state
    bool is_active() const;
    void set_active(bool active);

    void set_firing(bool firing);
    bool is_firing() const;

    // Pulse animation
    void set_pulse_enabled(bool enabled);
    bool is_pulse_enabled() const;
    void set_pulse_speed(float speed);
    float pulse_speed() const;

    // Shader
    void load_shader(const char* vs_path, const char* fs_path);
    void unload_shader();
    bool is_shader_loaded() const;

    // Update and draw
    void update(float dt);
    void draw() const;
};
