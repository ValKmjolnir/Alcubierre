#pragma once

#include "raylib.h"
#include "raymath.h"

// Light types
enum class light_type {
    point,
    directional,
    spot
};

// Base light interface
class light {
public:
    virtual ~light() = default;

    virtual light_type type() const = 0;
    virtual Vector3 position() const = 0;
    virtual Vector3 direction() const { return { 0.0f, -1.0f, 0.0f }; }
    virtual Vector3 color() const = 0;
    virtual float intensity() const = 0;
    virtual bool is_active() const { return true; }
    virtual void update(float dt) { (void)dt; }
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
