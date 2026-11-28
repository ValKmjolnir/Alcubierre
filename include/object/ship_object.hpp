#pragma once

#include <raylib.h>

#include <vector>

#include "object/object.hpp"

class mesh_renderer;

// Local space: nose along -Z, origin at the hull centre.
// titan — 1000 units long (10 km), macro blocks only
enum class hull_class {
    titan
};

class ship_object: public object {
public:
    struct part {
        Mesh mesh;
        Matrix transform;
        Color color;
        float spec_strength;
        float shininess;
    };

    ship_object(mesh_renderer& renderer, hull_class cls, const Vector3& position, float yaw_deg = 0.0f);
    ~ship_object() override;

    ship_object(const ship_object&) = delete;
    ship_object& operator=(const ship_object&) = delete;

    void set_pose(const Vector3& position, float yaw_deg) override;
    void translate(const Vector3& delta) override;
    void draw(const camera_3d& cam, int window_height) const override;
    bool blocks_line(const Vector3& from, const Vector3& to) const override;

    Vector3 cockpit_eye() const;
    Vector3 cockpit_forward() const;

    hull_class hull() const { return class_; }
    Vector3 position() const { return position_; }
    float yaw_deg() const { return yaw_deg_; }

private:
    void build_titan();
    void add(Mesh mesh, const Matrix& transform, Color color, float spec_strength, float shininess);

    mesh_renderer* renderer_;
    std::vector<part> parts_;
    hull_class class_;
    Vector3 position_;
    float yaw_deg_;
    Matrix model_;
};
