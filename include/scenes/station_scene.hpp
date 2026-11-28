#pragma once

#include "scenes/game_scene.hpp"

class station_scene: public game_scene {
public:
    explicit station_scene(lighting_system& lights);

    void build(game_window& window) override;
};
