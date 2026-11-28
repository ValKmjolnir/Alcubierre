#pragma once

#include "scenes/game_scene.hpp"
#include "ui/warp_anchor.hpp"

class fleet_scene: public game_scene {
public:
    explicit fleet_scene(lighting_system& lights);

    void build(mesh_renderer& renderer) override;
    const warp_anchor* anchor() const override { return &anchor_; }

private:
    warp_anchor anchor_;
};
