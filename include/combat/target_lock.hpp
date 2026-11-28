#pragma once

// Acquire timing for the currently selected target. Callers reset it when the
// target changes; it only owns elapsed time and the locked flag.
class target_lock {
public:
    static constexpr float kAcquireSeconds = 0.8f;

    void reset();

    // Advances the acquire timer; a locked target stays locked.
    void update(float dt);

    bool locked() const { return locked_; }
    float progress() const;

private:
    float elapsed_ = 0.0f;
    bool locked_ = false;
};
