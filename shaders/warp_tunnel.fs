#version 330

in vec2 fragTexCoord;

uniform float amount;
uniform float tunnelTime;

out vec4 fragColor;

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float value_noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 w = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, w.x), mix(c, d, w.x), w.y);
}

float fbm(vec2 p) {
    float sum = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        sum += amp * value_noise(p);
        p = p * 2.03 + vec2(17.3, 9.1);
        amp *= 0.5;
    }
    return sum;
}

void main() {
    float u = fragTexCoord.x;
    float v = fragTexCoord.y;

    float theta = u * 6.2831853;
    vec2 ring = vec2(cos(theta), sin(theta));

    float slow = fbm(ring * 1.6 + vec2(0.0, v * 2.2 + tunnelTime * 0.45));
    float mid = fbm(ring * 5.0 + vec2(11.0, v * 6.5 + tunnelTime * 1.15));
    float fast = fbm(ring * 11.0 + vec2(31.0, v * 13.0 + tunnelTime * 2.1));

    float field = slow * 0.45 + mid * 0.35 + fast * 0.20;
    float streak = smoothstep(0.38, 0.78, field);

    float pulse = 0.8 + 0.35 * value_noise(vec2(tunnelTime * 0.7, 4.3));

    float fade = smoothstep(0.0, 0.18, v) * (1.0 - smoothstep(0.65, 1.0, v));

    float intensity = streak * pulse * fade * amount * 0.55;
    vec3 color = mix(vec3(0.18, 0.32, 0.66), vec3(0.55, 0.70, 1.0), streak);
    fragColor = vec4(color, intensity);
}
