#pragma once

#include <raylib.h>

class grid {
private:
    Shader shader_ {};
    Material material_ {};
    Mesh mesh_ {};
    bool loaded_ = false;

    float spacing_ = 5.0f;
    float radius_ = 80.0f;
    Color color_ { 120, 200, 255, 255 };

    int loc_radius_ = -1;
    int loc_spacing_ = -1;
    int loc_color_ = -1;

public:
    grid(float spacing, float radius, Color color);
    ~grid();

    grid(const grid&) = delete;
    grid& operator=(const grid&) = delete;

    void draw(const Vector3& center) const;
};
