#pragma once

#include <raylib.h>

#include "object/object.hpp"

class game_window;

// Draws a .glb/.gltf through the engine's own lit shader instead of DrawModel,
// so imported ships keep the scene lighting and Blinn-Phong response.
// target_length rescales the model so its longest axis becomes that many world
// units, which doubles as the metre -> 10 m/unit conversion.
class model_object: public object {
public:
    model_object(game_window& win, const char* path, float target_length,
                 const Vector3& position, float yaw_deg = 0.0f);
    ~model_object() override;

    model_object(const model_object&) = delete;
    model_object& operator=(const model_object&) = delete;

    void draw(const camera_3d& cam, int window_height) const override;
    bool blocks_line(const Vector3& from, const Vector3& to) const override;

    bool loaded() const { return loaded_; }
    int mesh_count() const { return model_.meshCount; }

private:
    game_window* window_;
    Model model_ {};
    bool loaded_ = false;
    Matrix world_ {};
    BoundingBox bounds_ {};
};
