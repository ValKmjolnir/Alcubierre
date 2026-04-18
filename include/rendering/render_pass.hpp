#pragma once

#include "raylib.h"

#include <cstring>
#include <sstream>

#include "rendering/texture_handle.hpp"

class render_pass {
protected:
    std::string name;
    int width;
    int height;
    bool enabled = false;

    texture_handle output;

public:
    void set_enabled(bool flag) { enabled = flag; }
    bool is_enabled() const { return enabled; }

public:
    render_pass(const char* n, int w, int h):
        name(n), width(w), height(h) {}
    virtual void load();
    virtual void unload();
    virtual bool ready() const;
    virtual texture_handle& apply(const RenderTexture2D& texture, int width, int height);
};
