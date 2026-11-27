#include "ui/durability_style.hpp"

Color durability_layer_color(int index) {
    switch (index) {
    case 0: return Color { 0x58, 0xb4, 0xff, 255 };
    case 1: return Color { 0xff, 0xb4, 0x58, 255 };
    default: return Color { 0xff, 0x6a, 0x5a, 255 };
    }
}

const char* durability_layer_label(int index) {
    switch (index) {
    case 0: return "SHIELD";
    case 1: return "ARMOR";
    default: return "STRUCTURE";
    }
}

float durability_fraction(float current, float maximum) {
    if (maximum <= 0.0f) {
        return 0.0f;
    }
    const float fraction = current / maximum;
    if (fraction < 0.0f) {
        return 0.0f;
    }
    if (fraction > 1.0f) {
        return 1.0f;
    }
    return fraction;
}
