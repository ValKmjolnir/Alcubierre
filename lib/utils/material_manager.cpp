#include "utils/material_manager.hpp"

void safe_unload_material(Material& material) {
    material.shader = Shader { 0, nullptr };
    UnloadMaterial(material);
}