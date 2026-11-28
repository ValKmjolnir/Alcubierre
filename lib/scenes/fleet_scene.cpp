#include "scenes/fleet_scene.hpp"

#include <raymath.h>

#include "object/star.hpp"
#include "rendering/mesh_renderer.hpp"
#include "scenes/warp_path.hpp"
#include "ship/ship.hpp"
#include "ship/ship_class.hpp"

fleet_scene::fleet_scene(lighting_system& lights)
    : game_scene(lights)
    , anchor_(Vector3Scale(kWarpDirection, kWarpRouteDistance), "STATION") {}

void fleet_scene::build(mesh_renderer& renderer) {
    const Vector3 player_pos { 0.0f, 0.0f, 0.0f };
    const Vector3 escort_pos { -46.0f, -4.5f, -40.0f };
    const float escort_yaw = -60.0f;
    const Vector3 titan_pos { 110.0f, 40.0f, -1500.0f };
    const float titan_yaw = 105.0f;

    auto& main_star = objects_.add<star>(
        kFleetStarPosition, 255, 190, 155, 255);
    main_star.set_intensity(10.0f);
    main_star.set_scale(1000.0f);

    const auto ship_class_of = [](const char* id) -> const ship_class_def& {
        const ship_class_def* def = find_ship_class(id);
        if (!def) {
            TraceLog(LOG_FATAL, "unknown ship class %s", id);
        }
        return *def;
    };
    const auto weapon_of = [](const char* id) -> const weapon_def& {
        const weapon_def* def = find_weapon(id);
        if (!def) {
            TraceLog(LOG_FATAL, "unknown weapon %s", id);
        }
        return *def;
    };

    auto& player = objects_.add<ship>(
        renderer, ship_class_of("corvette_glb"), player_pos, 0.0f);
    player_ = &player;

    auto& escort = objects_.add<ship>(
        renderer, ship_class_of("corvette_glb"), escort_pos, escort_yaw);
    auto& titan = objects_.add<ship>(
        renderer, ship_class_of("titan_blockout"), titan_pos, titan_yaw);
    hostiles_.push_back(&escort);
    hostiles_.push_back(&titan);

    player.install_weapon(0, weapon_of("pulse_laser_s"));
    player.install_weapon(1, weapon_of("autocannon_s"));
    player.install_weapon(2, weapon_of("shield_restorer_s"));

    for (int i = 0; i < 5; i++) {
        escort.install_weapon(i, weapon_of("pulse_laser_s"));
    }
    escort.install_weapon(5, weapon_of("point_defence_s"));
    escort.install_weapon(6, weapon_of("point_defence_s"));

    titan.install_weapon(0, weapon_of("spinal_lance_c"));
    titan.install_weapon(1, weapon_of("beam_lance_c"));
    titan.install_weapon(2, weapon_of("beam_lance_c"));
    titan.install_weapon(3, weapon_of("flak_battery_c"));
    titan.install_weapon(4, weapon_of("flak_battery_c"));
    for (int i = 5; i < 9; i++) {
        titan.install_weapon(i, weapon_of("point_defence_c"));
    }

    player.aim_at(escort);
    escort.aim_at(player);
}
