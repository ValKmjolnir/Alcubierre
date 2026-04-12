#pragma once

#include "raylib.h"

// we now unload shader by shader_manager
// but raylib unload material with binded shader, which causes double free
// so we detach shader from material before unload
void safe_unload_material(Material& material);