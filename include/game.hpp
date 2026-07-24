#pragma once

#include "window.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "ui/menu.hpp"
#include "ui/debug_hud.hpp"
#include "utils/input_manager.hpp"
#include "object/laser_beam.hpp"
#include "object/projectile.hpp"
#include "object/star.hpp"

#include <vector>

class game {
private:
    // Declaration order matters — members are initialized in this order.
    // window_ must be first (creates OpenGL context needed by everything else).

    game_window window_;
    scene scene_;
    star& main_star_;

    camera_3d camera_;
    skybox sky_;
    menu menu_;
    debug_hud debug_hud_;
    input_manager input_;

    laser_beam beam_;
    std::vector<laser_beam> beams_;
    projectile proj1_;
    projectile proj2_;

public:
    game();
    void run();

private:
    void setup_beams();
    void setup_ui();
    void setup_post_processing();

    void handle_input(float dt);
    void update(float dt);
    void render();
};
