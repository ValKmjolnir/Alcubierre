#include "scenes/station_scene.hpp"

#include <raymath.h>

#include <cmath>

#include "object/star.hpp"
#include "object/station_object.hpp"
#include "rendering/mesh_renderer.hpp"
#include "scenes/scene_rebase.hpp"
#include "scenes/warp_path.hpp"
#include "ship/ship.hpp"
#include "ship/ship_class.hpp"

station_scene::station_scene(lighting_system& lights)
    : game_scene(lights)
    , fleet_anchor_(
        Vector3Scale(kWarpDirection, -kWarpRouteDistance), "FLEET") {}

void station_scene::build(mesh_renderer& renderer) {
    const scene_rebase star_rebase = scene_rebase::between(
        Vector3Scale(
            kWarpDirection,
            kWarpCorridorDistance - kWarpArrivalDistance),
        Vector3 { 0.0f, 0.0f, 0.0f });
    const Vector3 star_position = star_rebase.apply(kFleetStarPosition);

    auto& main_star = objects_.add<star>(
        star_position, 255, 190, 155, 255);
    main_star.set_intensity(10.0f);
    main_star.set_scale(1000.0f);

    objects_.add<station_object>(
        renderer,
        Vector3Scale(kWarpDirection, kWarpArrivalDistance),
        kWarpStationYawDeg);

    const ship_class_def* corvette = find_ship_class("corvette_glb");
    if (!corvette) {
        TraceLog(LOG_FATAL, "unknown ship class corvette_glb");
    }

    const float arrival_yaw = atan2f(-kWarpDirection.x, -kWarpDirection.z) * RAD2DEG;
    auto& player = objects_.add<ship>(
        renderer, *corvette, Vector3 { 0.0f, 0.0f, 0.0f }, arrival_yaw);
    player_ = &player;

    if (const weapon_def* weapon = find_weapon("pulse_laser_s")) {
        player.install_weapon(0, *weapon);
    }
    if (const weapon_def* weapon = find_weapon("autocannon_s")) {
        player.install_weapon(1, *weapon);
    }
    if (const weapon_def* weapon = find_weapon("shield_restorer_s")) {
        player.install_weapon(2, *weapon);
    }
}
