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
