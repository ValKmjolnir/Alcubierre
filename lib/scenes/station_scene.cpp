#include "scenes/station_scene.hpp"

#include "object/star.hpp"
#include "object/station_object.hpp"
#include "ship/ship.hpp"
#include "ship/ship_class.hpp"
#include "window.hpp"

station_scene::station_scene(lighting_system& lights)
    : game_scene(lights) {}

void station_scene::build(game_window& window) {
    auto& main_star = objects_.add<star>(
        Vector3 { 0.0f, 50.0f, 3000.0f }, 255, 190, 155, 255);
    main_star.set_intensity(10.0f);
    main_star.set_scale(1000.0f);

    objects_.add<station_object>(
        window, Vector3 { 250.0f, -30.0f, -1200.0f }, 20.0f);

    const ship_class_def* corvette = find_ship_class("corvette_glb");
    if (!corvette) {
        TraceLog(LOG_FATAL, "unknown ship class corvette_glb");
    }

    auto& player = objects_.add<ship>(
        window, *corvette, Vector3 { 0.0f, 0.0f, 0.0f }, 0.0f);
    player_ = &player;
}
