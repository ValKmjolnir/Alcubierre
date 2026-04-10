#pragma once

#include "raylib.h"
#include <cstring>
#include <sstream>

struct load_result {
    bool success;
    Shader shader;
};

load_result try_load_shader(const char* vs_path, const char* fs_path);