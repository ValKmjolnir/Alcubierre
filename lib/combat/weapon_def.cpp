#include "combat/weapon_def.hpp"

namespace {

const weapon_def kWeapons[] = {
    // id, name, category, size, delivery, damage, range, cooldown,
    // turret scale, barrels, barrel length, barrel radius,
    // colour, beam width, pulse speed, projectile speed, radius, lifetime
    { "pulse_laser_s", "Pulse Laser", weapon_category::offensive, weapon_size::small,
      weapon_delivery::beam, 16.0f, 900.0f, 0.5f,
      0.26f, 2, 2.6f, 0.10f,
      Color { 255, 150, 100, 255 }, 0.06f, 4.0f, 0.0f, 0.0f, 0.0f },

    { "autocannon_s", "Autocannon", weapon_category::offensive, weapon_size::small,
      weapon_delivery::projectile, 10.0f, 600.0f, 0.16f,
      0.24f, 3, 2.4f, 0.09f,
      Color { 255, 205, 120, 255 }, 0.0f, 0.0f, 220.0f, 0.05f, 2.2f },

    { "point_defence_s", "Point Defence", weapon_category::defensive, weapon_size::small,
      weapon_delivery::projectile, 5.0f, 300.0f, 0.10f,
      0.18f, 4, 2.0f, 0.07f,
      Color { 190, 220, 255, 255 }, 0.0f, 0.0f, 300.0f, 0.035f, 1.0f },

    { "shield_restorer_s", "Shield Restorer", weapon_category::support, weapon_size::small,
      weapon_delivery::utility, 0.0f, 700.0f, 8.0f,
      0.26f, 0, 0.0f, 0.0f,
      Color { 110, 170, 255, 255 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

    { "supply_feeder_s", "Supply Feeder", weapon_category::support, weapon_size::small,
      weapon_delivery::utility, 0.0f, 400.0f, 10.0f,
      0.22f, 0, 0.0f, 0.0f,
      Color { 150, 255, 170, 255 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

    { "railgun_m", "Railgun", weapon_category::offensive, weapon_size::medium,
      weapon_delivery::projectile, 110.0f, 1600.0f, 2.4f,
      0.62f, 2, 3.6f, 0.13f,
      Color { 255, 170, 90, 255 }, 0.0f, 0.0f, 320.0f, 0.10f, 6.0f },

    { "flak_battery_m", "Flak Battery", weapon_category::defensive, weapon_size::medium,
      weapon_delivery::projectile, 30.0f, 500.0f, 0.35f,
      0.55f, 4, 2.2f, 0.10f,
      Color { 255, 235, 180, 255 }, 0.0f, 0.0f, 240.0f, 0.05f, 1.6f },

    { "beam_lance_c", "Beam Lance", weapon_category::offensive, weapon_size::capital,
      weapon_delivery::beam, 900.0f, 3600.0f, 3.2f,
      18.0f, 1, 3.6f, 0.24f,
      Color { 150, 200, 255, 255 }, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f },

    { "spinal_lance_c", "Spinal Lance", weapon_category::offensive, weapon_size::capital,
      weapon_delivery::beam, 2400.0f, 5200.0f, 9.0f,
      20.0f, 1, 8.5f, 0.25f,
      Color { 120, 230, 255, 255 }, 1.6f, 0.0f, 0.0f, 0.0f, 0.0f },

    { "flak_battery_c", "Capital Flak Battery", weapon_category::defensive, weapon_size::large,
      weapon_delivery::projectile, 90.0f, 1400.0f, 0.30f,
      5.0f, 4, 2.2f, 0.08f,
      Color { 255, 225, 160, 255 }, 0.0f, 0.0f, 400.0f, 0.20f, 2.5f },

    { "point_defence_c", "Capital Point Defence", weapon_category::defensive, weapon_size::small,
      weapon_delivery::projectile, 20.0f, 900.0f, 0.12f,
      2.2f, 4, 2.4f, 0.09f,
      Color { 200, 230, 255, 255 }, 0.0f, 0.0f, 500.0f, 0.08f, 1.2f },

    { "shield_restorer_c", "Capital Shield Restorer", weapon_category::support, weapon_size::large,
      weapon_delivery::utility, 0.0f, 3000.0f, 12.0f,
      6.0f, 0, 0.0f, 0.0f,
      Color { 110, 170, 255, 255 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
};

constexpr int kWeaponCount = static_cast<int>(sizeof(kWeapons) / sizeof(kWeapons[0]));

} // namespace

int weapon_size_rank(weapon_size size) {
    switch (size) {
    case weapon_size::small: return 0;
    case weapon_size::medium: return 1;
    case weapon_size::large: return 2;
    case weapon_size::capital: return 3;
    }
    return 0;
}

bool weapon_fits(weapon_size slot, weapon_size weapon) {
    return weapon_size_rank(weapon) <= weapon_size_rank(slot);
}

const weapon_def* weapon_catalog() {
    return kWeapons;
}

int weapon_catalog_size() {
    return kWeaponCount;
}

const weapon_def* find_weapon(const char* id) {
    for (int i = 0; i < kWeaponCount; i++) {
        if (TextIsEqual(kWeapons[i].id, id)) {
            return &kWeapons[i];
        }
    }
    return nullptr;
}
