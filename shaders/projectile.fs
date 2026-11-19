#version 330

in vec2 vLocal;

uniform vec4 color;         // Projectile color (rgb normalized) and age fade (a)
uniform float intensity;    // Glow brightness multiplier
uniform float coreFrac;     // Saturated core radius as a fraction of the billboard
uniform float projAge;
uniform float projLifetime;

out vec4 fragColor;

void main()
{
    const vec3 hotColor = vec3(1.0, 0.95, 0.88);

    float d = length(vLocal);

    float glow = 0.0;
    if (d <= coreFrac) {
        glow = 1.0;
    } else if (d < 1.0) {
        float t = (d - coreFrac) / (1.0 - coreFrac);
        glow = (1.0 - t) * (1.0 - t);
    }

    float ageFade = 1.0;
    if (projAge > projLifetime * 0.75) {
        ageFade = clamp(1.0 - projAge / projLifetime, 0.0, 1.0) * 1.33;
    }

    float brightness = glow * intensity * ageFade * color.a;
    float hot = pow(glow, 2.5) * clamp(intensity * 0.6, 0.0, 1.0);

    vec3 rgb = mix(color.rgb, hotColor, hot) * brightness;

    fragColor = vec4(rgb, 1.0);
}
