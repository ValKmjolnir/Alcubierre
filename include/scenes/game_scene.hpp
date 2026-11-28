#pragma once

#include <vector>

#include "scene.hpp"

class game_window;
class ship;
class warp_anchor;

class game_scene {
public:
    explicit game_scene(lighting_system& lights);

    virtual ~game_scene() = default;

    virtual void build(game_window& window) = 0;

    virtual void update(float dt);
    virtual void draw(const camera_3d& camera, int window_height) const;
    virtual Vector2 draw_flares(const camera_3d& camera, int rt_width,
                                int rt_height, int window_height) const;

    ship* player() const { return player_; }
    const std::vector<ship*>& hostiles() const { return hostiles_; }
    virtual const warp_anchor* anchor() const { return nullptr; }

protected:
    scene objects_;
    ship* player_ = nullptr;
    std::vector<ship*> hostiles_;
};
