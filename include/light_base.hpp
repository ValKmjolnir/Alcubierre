#pragma once

#include <raylib.h>
#include <raymath.h>

// Light types
enum class light_type {
    point,
    directional,
    spot,
    ambient
};

// Base light interface
class light {
public:
    virtual ~light() = default;

    virtual light_type type() const = 0;
    virtual Vector3 position() const = 0;
    virtual Vector3 direction() const { return { 0.0f, -1.0f, 0.0f }; }
    virtual Vector3 color() const = 0;
    virtual Vector3 ground_color() const { return color(); }
    virtual float intensity() const = 0;
    virtual float cutoff() const { return 1.0f; }
    virtual float inner_cutoff() const { return 1.0f; }
    virtual float range() const { return 0.0f; }
    virtual bool is_active() const { return true; }
};

// Point light - emits in all directions
class point_light : public light {
public:
    point_light();
    point_light(const Vector3& position, const Vector3& color, float intensity);

    light_type type() const override { return light_type::point; }
    Vector3 position() const override { return position_; }
    Vector3 color() const override { return color_; }
    float intensity() const override { return intensity_; }

    void set_position(const Vector3& pos) { position_ = pos; }
    void set_color(const Vector3& col) { color_ = col; }
    void set_intensity(float intensity) { intensity_ = intensity; }
    void set_active(bool active) { active_ = active; }
    bool is_active() const override { return active_; }

protected:
    Vector3 position_;
    Vector3 color_;
    float intensity_;
    bool active_;
};

// Ambient light - constant sky/ground illumination, no position
class ambient_light : public light {
public:
    ambient_light();
    ambient_light(const Vector3& sky_color, const Vector3& ground_color, float intensity);

    light_type type() const override { return light_type::ambient; }
    Vector3 position() const override { return { 0.0f, 0.0f, 0.0f }; }
    Vector3 color() const override { return sky_color_; }
    Vector3 ground_color() const override { return ground_color_; }
    float intensity() const override { return intensity_; }

    void set_sky_color(const Vector3& color) { sky_color_ = color; }
    void set_ground_color(const Vector3& color) { ground_color_ = color; }
    void set_intensity(float intensity) { intensity_ = intensity; }
    void set_active(bool active) { active_ = active; }
    bool is_active() const override { return active_; }

private:
    Vector3 sky_color_;
    Vector3 ground_color_;
    float intensity_;
    bool active_;
};

// Spot light - point light restricted to a cone
class spot_light : public light {
public:
    spot_light();
    spot_light(const Vector3& position, const Vector3& direction, const Vector3& color,
               float intensity, float inner_angle_deg, float outer_angle_deg,
               float range = 10.0f);

    light_type type() const override { return light_type::spot; }
    Vector3 position() const override { return position_; }
    Vector3 direction() const override { return direction_; }
    Vector3 color() const override { return color_; }
    float intensity() const override { return intensity_; }
    float cutoff() const override { return outer_cutoff_; }
    float inner_cutoff() const override { return inner_cutoff_; }
    float range() const override { return range_; }

    void set_position(const Vector3& position) { position_ = position; }
    void set_direction(const Vector3& direction) { direction_ = direction; }
    void set_color(const Vector3& color) { color_ = color; }
    void set_intensity(float intensity) { intensity_ = intensity; }
    void set_angles(float inner_angle_deg, float outer_angle_deg);
    void set_range(float range) { range_ = range; }
    void set_active(bool active) { active_ = active; }
    bool is_active() const override { return active_; }

private:
    Vector3 position_;
    Vector3 direction_;
    Vector3 color_;
    float intensity_;
    float inner_cutoff_;
    float outer_cutoff_;
    float range_;
    bool active_;
};
