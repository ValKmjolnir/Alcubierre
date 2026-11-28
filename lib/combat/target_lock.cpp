#include "combat/target_lock.hpp"

void target_lock::reset() {
    elapsed_ = 0.0f;
    locked_ = false;
}

void target_lock::update(float dt) {
    if (locked_) {
        return;
    }

    elapsed_ += dt;
    if (elapsed_ >= kAcquireSeconds) {
        elapsed_ = kAcquireSeconds;
        locked_ = true;
    }
}

float target_lock::progress() const {
    if (locked_) {
        return 1.0f;
    }
    return elapsed_ / kAcquireSeconds;
}
