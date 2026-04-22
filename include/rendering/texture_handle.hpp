#pragma once

#include <raylib.h>

class texture_handle {
private:
    RenderTexture2D target;
    bool initialized = false;

public:
    texture_handle() = default;
    ~texture_handle() = default;

    void load(int w, int h);
    void set(int w, int h);
    void unload();

    bool ready() const { return initialized; }
    auto& get() { return target; }
};