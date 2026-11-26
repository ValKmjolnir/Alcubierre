#version 330

// Tactical grid drawn on a quad lying in the XZ plane. Rings and spokes are
// derived from the local position, anti-aliased with screen-space derivatives
// and faded out at the edge and under the ship.

in vec2 fragLocal;

uniform float gridRadius;
uniform float gridSpacing;
uniform vec4 gridColor;

out vec4 finalColor;

float ring_line(float dist, float spacing) {
    float nearest = floor(dist / spacing + 0.5) * spacing;
    float offset = abs(dist - nearest);
    float width = fwidth(dist) * 1.5;
    return 1.0 - smoothstep(0.0, width, offset);
}

float spoke_line(vec2 p) {
    const float spokeAngle = 0.2617994; // 15 degrees
    float angle = atan(p.y, p.x);
    float nearest = floor(angle / spokeAngle + 0.5) * spokeAngle;
    float offset = abs(angle - nearest);
    float width = fwidth(angle) * 1.5;
    return 1.0 - smoothstep(0.0, width, offset);
}

void main() {
    float dist = length(fragLocal);

    float minor = ring_line(dist, gridSpacing);
    float major = ring_line(dist, gridSpacing * 5.0);
    float spoke = spoke_line(fragLocal);

    float edge_fade = 1.0 - smoothstep(gridRadius * 0.7, gridRadius, dist);
    float center_fade = smoothstep(0.0, gridSpacing * 0.5, dist);
    float fade = edge_fade * center_fade;

    float coverage = minor * 0.30 + major * 0.55 + spoke * 0.25;
    coverage = clamp(coverage, 0.0, 1.0) * fade;

    finalColor = vec4(gridColor.rgb, gridColor.a * coverage);
}
