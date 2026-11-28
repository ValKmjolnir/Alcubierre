#pragma once

#include "scenes/game_scene.hpp"
#include "ui/warp_anchor.hpp"

class station_scene: public game_scene {
public:
    explicit station_scene(lighting_system& lights);

    void build(game_window& window) override;
    const warp_anchor* anchor() const override { return &fleet_anchor_; }
    bool warp_to_fleet() const override { return true; }

private:
    warp_anchor fleet_anchor_;
};
