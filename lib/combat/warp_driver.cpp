#include "combat/warp_driver.hpp"

#include <raylib.h>
#include <raymath.h>

#include <cmath>

#include "rendering/warp.hpp"
#include "scenes/warp_path.hpp"
#include "ship/ship.hpp"

namespace {

constexpr float kAlignTurnDegPerSec = 30.0f;

constexpr float kWarpAccelSeconds = 1.3f;
constexpr float kWarpCruiseSeconds = 0.9f;
constexpr float kWarpDecelSeconds = 1.3f;
constexpr float kWarpSpoolSeconds =
    kWarpAccelSeconds + kWarpCruiseSeconds + kWarpDecelSeconds;
constexpr float kWarpArrivalSeconds = 3.5f;
constexpr float kWarpArrivalHoldSeconds = 0.5f;
constexpr float kWarpPeakFactor = 4.5f;
constexpr float kWarpPeakBeta = 0.9f;
constexpr float kWarpArrivalResidual = 0.35f;

constexpr float kWarpSpeedArea =
    kWarpAccelSeconds * 0.5f + kWarpCruiseSeconds + kWarpDecelSeconds * 0.5f;

float smooth01(float value) {
    value = Clamp(value, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

float warp_speed_shape(float elapsed) {
    if (elapsed <= 0.0f || elapsed >= kWarpSpoolSeconds) {
        return 0.0f;
    }
    if (elapsed < kWarpAccelSeconds) {
        return smooth01(elapsed / kWarpAccelSeconds);
    }
    if (elapsed < kWarpAccelSeconds + kWarpCruiseSeconds) {
        return 1.0f;
    }
    const float u = (elapsed - kWarpAccelSeconds - kWarpCruiseSeconds)
        / kWarpDecelSeconds;
    return 1.0f - smooth01(u);
}

float warp_progress(float elapsed) {
    if (elapsed <= 0.0f) {
        return 0.0f;
    }
    if (elapsed >= kWarpSpoolSeconds) {
        return 1.0f;
    }
    if (elapsed < kWarpAccelSeconds) {
        const float u = elapsed / kWarpAccelSeconds;
        const float u2 = u * u;
        return kWarpAccelSeconds * (u2 * u - 0.5f * u2 * u2)
            / kWarpSpeedArea;
    }
    if (elapsed < kWarpAccelSeconds + kWarpCruiseSeconds) {
        return (kWarpAccelSeconds * 0.5f
            + (elapsed - kWarpAccelSeconds)) / kWarpSpeedArea;
    }
    const float u = (elapsed - kWarpAccelSeconds - kWarpCruiseSeconds)
        / kWarpDecelSeconds;
    const float speed_integral = u - u * u * u + 0.5f * u * u * u * u;
    return (kWarpAccelSeconds * 0.5f + kWarpCruiseSeconds
        + kWarpDecelSeconds * speed_integral) / kWarpSpeedArea;
}

Vector3 ship_forward(const ship& s) {
    return Vector3Transform(
        Vector3 { 0.0f, 0.0f, -1.0f },
        MatrixRotateY(s.yaw_deg() * DEG2RAD));
}

} // namespace

void warp_driver::reset() {
    aligning_ = false;
    warp_after_alignment_ = false;
    align_target_yaw_ = 0.0f;
    anchor_ = {};

    stage_ = stage::idle;
    stage_time_ = 0.0f;
    start_position_ = {};
    end_position_ = {};
    to_fleet_ = false;
}

Vector3 warp_driver::corridor_end_position(const Vector3& player, const Vector3& anchor) {
    const Vector3 to_anchor = Vector3Subtract(anchor, player);
    const float distance = Vector3Length(to_anchor);
    if (distance < 0.001f) {
        return player;
    }

    return Vector3Add(
        player,
        Vector3Scale(
            to_anchor,
            (kWarpCorridorDistance - kWarpArrivalDistance) / distance));
}

bool warp_driver::start_alignment(const ship& player, const Vector3& anchor,
                                  bool warp_after, bool to_fleet) {
    if (stage_ != stage::idle) {
        return false;
    }

    const Vector3 direction = Vector3Subtract(anchor, player.position());
    if (fabsf(direction.x) < 1e-4f && fabsf(direction.z) < 1e-4f) {
        return false;
    }

    anchor_ = anchor;
    warp_after_alignment_ = warp_after;
    to_fleet_ = to_fleet;
    align_target_yaw_ = atan2f(-direction.x, -direction.z) * RAD2DEG;
    aligning_ = true;
    TraceLog(LOG_INFO, "anchor: aligning to %.1f deg", align_target_yaw_);
    return true;
}

bool warp_driver::begin_warp(ship& player, warp_renderer& warp) {
    const Vector3 to_anchor = Vector3Subtract(anchor_, player.position());
    const float distance = Vector3Length(to_anchor);
    if (distance < 0.001f) {
        return false;
    }
    warp.set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    warp.set_warp_factor(0.0f);

    start_position_ = player.position();
    end_position_ = corridor_end_position(start_position_, anchor_);

    stage_ = stage::spooling;
    stage_time_ = 0.0f;
    TraceLog(LOG_INFO, "warp: spooling");
    return true;
}

warp_driver::event warp_driver::update(float dt, ship& player,
                                       warp_renderer& warp) {
    if (aligning_) {
        if (!player.align_yaw_to(
                align_target_yaw_, kAlignTurnDegPerSec, dt)) {
            return event::none;
        }

        aligning_ = false;
        TraceLog(LOG_INFO, "anchor: alignment complete");

        if (!warp_after_alignment_) {
            return event::none;
        }
        warp_after_alignment_ = false;
        TraceLog(LOG_INFO, "anchor: warp after alignment");
        return begin_warp(player, warp) ? event::started : event::none;
    }

    if (stage_ == stage::idle) {
        return event::none;
    }

    stage_time_ += dt;

    if (stage_ == stage::spooling) {
        const float elapsed = stage_time_;
        const float progress = warp_progress(elapsed);
        const float speed = warp_speed_shape(elapsed);
        const float attack = smooth01(elapsed / kWarpAccelSeconds);
        const float envelope = attack
            * (kWarpArrivalResidual + (1.0f - kWarpArrivalResidual) * speed);
        warp.set_warp_factor(kWarpPeakFactor * envelope);
        warp.set_velocity(Vector3Scale(
            ship_forward(player), kWarpPeakBeta * envelope));
        player.set_position(Vector3Add(
            start_position_,
            Vector3Scale(
                Vector3Subtract(end_position_, start_position_),
                progress)));

        if (elapsed < kWarpSpoolSeconds) {
            return event::none;
        }

        stage_ = stage::idle;
        stage_time_ = 0.0f;
        return event::jump;
    }

    const float fade_t = Clamp(
        (stage_time_ - kWarpArrivalHoldSeconds)
            / (kWarpArrivalSeconds - kWarpArrivalHoldSeconds),
        0.0f, 1.0f);
    const float s = smooth01(fade_t);
    warp.set_warp_factor(
        kWarpPeakFactor * kWarpArrivalResidual * (1.0f - s));
    warp.set_velocity(Vector3Scale(
        ship_forward(player),
        kWarpPeakBeta * kWarpArrivalResidual * (1.0f - s)));

    if (fade_t < 1.0f) {
        return event::none;
    }

    stage_ = stage::idle;
    stage_time_ = 0.0f;
    warp.set_warp_factor(0.0f);
    warp.set_velocity(Vector3 { 0.0f, 0.0f, 0.0f });
    TraceLog(LOG_INFO, "warp: arrival complete");
    return event::finished;
}

void warp_driver::begin_arrival(ship& player, warp_renderer& warp) {
    stage_ = stage::arriving;
    stage_time_ = 0.0f;
    warp.set_warp_factor(kWarpPeakFactor * kWarpArrivalResidual);
    warp.set_velocity(Vector3Scale(
        ship_forward(player),
        kWarpPeakBeta * kWarpArrivalResidual));
}

float warp_driver::tunnel_amount() const {
    if (stage_ != stage::spooling) {
        return 0.0f;
    }

    const float p = Clamp(stage_time_ / kWarpSpoolSeconds, 0.0f, 1.0f);
    const float ramp_in = smooth01(p / 0.15f);
    const float ramp_out = 1.0f - smooth01((p - 0.55f) / 0.40f);
    return ramp_in * ramp_out;
}
