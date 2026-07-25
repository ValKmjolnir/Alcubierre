#pragma once

#include "window.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "ui/menu.hpp"
#include "ui/debug_hud.hpp"
#include "utils/input_manager.hpp"

class game {
private:
    game_window window_;
    scene scene_;
    camera_3d camera_;
    skybox sky_;
    menu menu_;
    debug_hud debug_hud_;
    input_manager input_;

public:
    game();
    void run();

private:
    void setup_scene();
    void setup_ui();
    void setup_post_processing();

    void handle_input(float dt);
    void update(float dt);
    void render();
};
