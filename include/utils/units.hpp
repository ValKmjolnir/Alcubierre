#pragma once

// World unit convention: 1 unit = 10 m, so 100 units = 1 km.
constexpr float kUnitsPerKm = 100.0f;
constexpr double kKmPerAu = 149597870.7;

constexpr float units_to_km(float units) {
    return units / kUnitsPerKm;
}
