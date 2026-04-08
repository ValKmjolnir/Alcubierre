#version 330

// ============================================================
// Lighting utility functions for GLSL shaders
// Include this file in any shader that needs lighting calculations.
// Usage: #include "lighting.glsl"  (or paste contents directly)
// ============================================================

#define MAX_LIGHTS 8
#define LIGHT_TYPE_POINT      0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_SPOT       2

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float type;           // 0=point, 1=directional, 2=spot
    vec3 direction;       // For directional/spot lights
    float cutoff;         // For spot lights
};

uniform Light lights[MAX_LIGHTS];
uniform int light_count;

// Calculate diffuse contribution from a single point light
vec3 calculate_point_light_diffuse(
    vec3 normal,
    vec3 fragPos,
    vec3 lightPos,
    vec3 lightColor,
    float lightIntensity
) {
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    return diff * lightColor * lightIntensity;
}

// Calculate diffuse contribution from a single directional light
vec3 calculate_directional_light_diffuse(
    vec3 normal,
    vec3 lightDir,
    vec3 lightColor,
    float lightIntensity
) {
    float diff = max(dot(normal, normalize(lightDir)), 0.0);
    return diff * lightColor * lightIntensity;
}

// Calculate diffuse + specular from a single point light (Blinn-Phong)
vec3 calculate_point_light_full(
    vec3 normal,
    vec3 fragPos,
    vec3 viewDir,
    vec3 lightPos,
    vec3 lightColor,
    float lightIntensity,
    float shininess
) {
    vec3 lightDir = normalize(lightPos - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = spec * lightColor * lightIntensity * 0.3;

    return diffuse + specular;
}

// Calculate lighting from all active lights (diffuse only)
vec3 calculate_all_lights_diffuse(
    vec3 normal,
    vec3 fragPos,
    vec3 baseColor
) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
        if (lights[i].type == LIGHT_TYPE_POINT) {
            vec3 diff = calculate_point_light_diffuse(
                normal, fragPos,
                lights[i].position,
                lights[i].color,
                lights[i].intensity
            );
            result += diff * baseColor;
        } else if (lights[i].type == LIGHT_TYPE_DIRECTIONAL) {
            vec3 diff = calculate_directional_light_diffuse(
                normal,
                lights[i].position, // For directional, position is actually direction
                lights[i].color,
                lights[i].intensity
            );
            result += diff * baseColor;
        }
    }

    return result;
}

// Calculate lighting from all active lights (full Blinn-Phong)
vec3 calculate_all_lights_full(
    vec3 normal,
    vec3 fragPos,
    vec3 viewDir,
    vec3 baseColor,
    float shininess
) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
        if (lights[i].type == LIGHT_TYPE_POINT) {
            result += calculate_point_light_full(
                normal, fragPos, viewDir,
                lights[i].position,
                lights[i].color,
                lights[i].intensity,
                shininess
            ) * baseColor;
        }
    }

    return result;
}
