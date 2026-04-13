#version 330

// SMAA (Subpixel Morphological Anti-Aliasing) - Fragment Shader
// Pass 3: Neighborhood Blending / Resolve
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;   // Original scene texture
uniform sampler2D texture1;   // Blend weight texture from Pass 2
uniform vec2 resolution;

void main()
{
    vec2 uv = fragTexCoord;
    vec2 texelSize = 1.0 / resolution;

    // Read blend weights from Pass 2
    vec4 weights = texture(texture1, uv);
    float blendH = weights.r;  // Position in horizontal edge (0-1)
    float blendV = weights.g;  // Position in vertical edge (0-1)
    float edgeStrength = weights.b;

    // No edge → return original color
    if (edgeStrength < 0.01)
    {
        fragColor = texture(texture0, uv);
        return;
    }

    // Sample the original color and neighbors
    vec3 center = texture(texture0, uv).rgb;
    vec3 colorN = texture(texture0, uv + vec2(0.0, -texelSize.y)).rgb;
    vec3 colorS = texture(texture0, uv + vec2(0.0,  texelSize.y)).rgb;
    vec3 colorE = texture(texture0, uv + vec2( texelSize.x, 0.0)).rgb;
    vec3 colorW = texture(texture0, uv + vec2(-texelSize.x, 0.0)).rgb;

    vec3 color = center;

    // For horizontal edges: blend center with vertical neighbors (N/S)
    // The blend weight tells us where we are in the edge segment
    // Maximum blending at center of edge (blendH = 0.5), less at endpoints
    if (blendH > 0.01)
    {
        float positionFactor = 1.0 - abs(blendH - 0.5) * 2.0;  // 1.0 at center, 0.0 at endpoints
        float blendAmount = edgeStrength * positionFactor;
        vec3 blendedNS = mix(colorN, colorS, blendH);
        color = mix(center, blendedNS, blendAmount);
    }

    // For vertical edges: blend center with horizontal neighbors (E/W)
    if (blendV > 0.01)
    {
        float positionFactor = 1.0 - abs(blendV - 0.5) * 2.0;
        float blendAmount = edgeStrength * positionFactor;
        vec3 blendedEW = mix(colorE, colorW, blendV);
        color = mix(color, blendedEW, blendAmount);
    }

    fragColor = vec4(color, 1.0);
}
