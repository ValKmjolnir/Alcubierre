#pragma once

#include <raylib.h>

class ship;
class warp_renderer;

class warp_driver {
public:
    enum class event {
        none,
        started,
        jump,
        finished
    };

    void reset();

    static Vector3 destination_position(const Vector3& player, const Vector3& anchor);

    bool start_alignment(const ship& player, const Vector3& anchor,
                         bool warp_after, bool to_fleet);
    event update(float dt, ship& player, warp_renderer& warp);
    void begin_arrival(ship& player, warp_renderer& warp);

    bool active() const { return stage_ != stage::idle; }
    bool to_fleet() const { return to_fleet_; }

private:
    enum class stage {
        idle,
        spooling,
        arriving
    };

    bool begin_warp(ship& player, warp_renderer& warp);

    bool aligning_ = false;
    bool warp_after_alignment_ = false;
    float align_target_yaw_ = 0.0f;
    Vector3 anchor_ {};

    stage stage_ = stage::idle;
    float stage_time_ = 0.0f;
    Vector3 start_position_ {};
    Vector3 end_position_ {};
    bool to_fleet_ = false;
};
