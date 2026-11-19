#version 330

// Input fragment attributes
in vec3 fragPosition;
in vec3 fragNormal;

// Input uniform attributes
uniform vec3 objectColor;       // Base color of the object
uniform vec3 viewPos;           // Camera position, for specular
uniform float specStrength;     // Per-object specular amount
uniform float shininess;        // Per-object highlight tightness

// Lighting system uniforms
#define MAX_LIGHTS 8

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float type;
    vec3 direction;
    float cutoff;
    vec3 groundColor;
    float innerCutoff;
    float range;
};

uniform Light lights[MAX_LIGHTS];
uniform int light_count;

// Output fragment color
out vec4 fragColor;

float specular_factor(vec3 normal, vec3 lightDir)
{
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 halfDir = normalize(lightDir + viewDir);
    float ndoth = max(dot(normal, halfDir), 0.0);
    return pow(ndoth, shininess) * specStrength;
}

void main()
{
    vec3 normal = normalize(fragNormal);

    vec3 lighting = vec3(0.0);

    for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
        int type = int(lights[i].type + 0.5);

        if (type == 0) {
            vec3 lightDir = normalize(lights[i].position - fragPosition);
            float diff = max(dot(normal, lightDir), 0.0);
            float spec = specular_factor(normal, lightDir);
            lighting += (diff * objectColor + spec) * lights[i].color * lights[i].intensity;
        } else if (type == 1) {
            vec3 lightDir = normalize(-lights[i].direction);
            float diff = max(dot(normal, lightDir), 0.0);
            float spec = specular_factor(normal, lightDir);
            lighting += (diff * objectColor + spec) * lights[i].color * lights[i].intensity;
        } else if (type == 2) {
            vec3 lightDir = normalize(lights[i].position - fragPosition);
            float diff = max(dot(normal, lightDir), 0.0);
            float spec = specular_factor(normal, lightDir);
            float theta = dot(lightDir, normalize(-lights[i].direction));
            float cone = smoothstep(lights[i].cutoff, lights[i].innerCutoff, theta);
            float dist = length(lights[i].position - fragPosition);
            float atten = 1.0 / (1.0 + dist * dist);
            float window = clamp(1.0 - pow(dist / max(lights[i].range, 1e-4), 4.0), 0.0, 1.0);
            atten *= window * window;
            lighting += (diff * objectColor + spec) * cone * atten * lights[i].color * lights[i].intensity;
        } else if (type == 3) {
            float hemi = normal.y * 0.5 + 0.5;
            lighting += mix(lights[i].groundColor, lights[i].color, hemi) * lights[i].intensity * objectColor;
        }
    }

    // Simple tone mapping to avoid over-bright areas
    vec3 finalColor = lighting / (lighting + vec3(1.0));

    fragColor = vec4(finalColor, 1.0);
}
