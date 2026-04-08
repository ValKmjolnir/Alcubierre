#version 330

// Input fragment attributes
in vec3 fragPosition;
in vec3 fragNormal;

// Input uniform attributes
uniform vec3 objectColor;       // Base color of the object
uniform float ambientStrength;  // Ambient light intensity

// Lighting system uniforms
#define MAX_LIGHTS 8

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float type;
    vec3 direction;
    float cutoff;
};

uniform Light lights[MAX_LIGHTS];
uniform int light_count;

// Output fragment color
out vec4 fragColor;

void main()
{
    vec3 normal = normalize(fragNormal);
    vec3 ambient = objectColor * ambientStrength;

    // Accumulate diffuse lighting from all active lights
    vec3 diffuse = vec3(0.0);

    for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
        vec3 lightDir = normalize(lights[i].position - fragPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        diffuse += diff * lights[i].color * lights[i].intensity * objectColor;
    }

    // Final color = ambient + diffuse (no specular for now)
    vec3 finalColor = ambient + diffuse;

    // Simple tone mapping to avoid over-bright areas
    finalColor = finalColor / (finalColor + vec3(1.0));

    fragColor = vec4(finalColor, 1.0);
}
