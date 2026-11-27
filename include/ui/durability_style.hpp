#pragma once

#include <raylib.h>

Color durability_layer_color(int index);
const char* durability_layer_label(int index);
float durability_fraction(float current, float maximum);
