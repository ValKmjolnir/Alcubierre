#pragma once

#include <raylib.h>

#include <vector>

#include "object/object.hpp"

class game_window;

// Blockout hull: 12 x 5 x 2.6, origin at geometric center, nose along -Z.
class ship_object: public object {
public:
    struct part {
        Mesh mesh;
        Matrix transform;
        Color color;
        float spec_strength;
        float shininess;
    };

    explicit ship_object(game_window& win);
    ~ship_object() override;

    ship_object(const ship_object&) = delete;
    ship_object& operator=(const ship_object&) = delete;

    void draw(const camera_3d& cam, int window_height) const override;
    bool blocks_line(const Vector3& from, const Vector3& to) const override;

    Vector3 cockpit_eye() const { return Vector3 { 0.0f, 1.0f, -3.8f }; }
    Vector3 cockpit_forward() const { return Vector3 { 0.0f, 0.0f, -1.0f }; }

private:
    void build();
    void add(Mesh mesh, const Matrix& transform, Color color, float spec_strength, float shininess);

    game_window* window_;
    std::vector<part> parts_;
};
