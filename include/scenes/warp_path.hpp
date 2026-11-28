#pragma once

#include <raylib.h>

constexpr Vector3 kWarpDirection { -0.5144958f, 0.0f, -0.8574929f };
constexpr Vector3 kFleetStarPosition { 0.0f, 50.0f, 3000.0f };
constexpr float kWarpUnitsPerAu = 14959787070.0f;
constexpr float kWarpRouteDistance = 10.0f * kWarpUnitsPerAu;
constexpr float kWarpCorridorDistance = 20000.0f;
constexpr float kWarpArrivalDistance = 1200.0f;
constexpr float kWarpStationYawDeg = 20.0f;
