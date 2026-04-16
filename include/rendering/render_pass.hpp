#pragma once

#include "raylib.h"

#include <cstring>
#include <sstream>

class render_pass {
private:
    std::string name;

public:
    render_pass(const char* n): name(n) {}
};