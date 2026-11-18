#include "light_base.hpp"

point_light::point_light()
    : position_{ 0.0f, 0.0f, 0.0f }
    , color_{ 1.0f, 1.0f, 1.0f }
    , intensity_(1.0f)
    , active_(true)
{
}

point_light::point_light(const Vector3& position, const Vector3& color, float intensity)
    : position_(position)
    , color_(color)
    , intensity_(intensity)
    , active_(true)
{
}

ambient_light::ambient_light()
    : sky_color_{ 0.0f, 0.0f, 0.0f }
    , ground_color_{ 0.0f, 0.0f, 0.0f }
    , intensity_(1.0f)
    , active_(true)
{
}

ambient_light::ambient_light(const Vector3& sky_color, const Vector3& ground_color, float intensity)
    : sky_color_(sky_color)
    , ground_color_(ground_color)
    , intensity_(intensity)
    , active_(true)
{
}

spot_light::spot_light()
    : position_{ 0.0f, 0.0f, 0.0f }
    , direction_{ 0.0f, -1.0f, 0.0f }
    , color_{ 1.0f, 1.0f, 1.0f }
    , intensity_(1.0f)
    , inner_cutoff_(cosf(20.0f * DEG2RAD))
    , outer_cutoff_(cosf(30.0f * DEG2RAD))
    , range_(10.0f)
    , active_(true)
{
}

spot_light::spot_light(const Vector3& position, const Vector3& direction, const Vector3& color,
                       float intensity, float inner_angle_deg, float outer_angle_deg,
                       float range)
    : position_(position)
    , direction_(direction)
    , color_(color)
    , intensity_(intensity)
    , inner_cutoff_(0.0f)
    , outer_cutoff_(0.0f)
    , range_(range)
    , active_(true)
{
    set_angles(inner_angle_deg, outer_angle_deg);
}

void spot_light::set_angles(float inner_angle_deg, float outer_angle_deg) {
    if (outer_angle_deg < inner_angle_deg) {
        outer_angle_deg = inner_angle_deg;
    }
    inner_cutoff_ = cosf(inner_angle_deg * DEG2RAD);
    outer_cutoff_ = cosf(outer_angle_deg * DEG2RAD);
}
