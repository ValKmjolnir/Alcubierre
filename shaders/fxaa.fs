#version 330

// FXAA (Fast Approximate Anti-Aliasing) - Fragment Shader
// Based on NVIDIA FXAA 3.11 by Timothy Lottes
// Adapted for GLSL 330
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;  // Screen resolution (width, height)

// FXAA quality presets
// Lower values = faster, higher values = better quality
const float FXAA_SPAN_MAX = 8.0;
const float FXAA_REDUCE_MUL = 1.0 / 8.0;
const float FXAA_REDUCE_MIN = 1.0 / 128.0;

// Luminance weights (human eye sensitivity)
const vec3 LUM = vec3(0.299, 0.587, 0.114);

// Convert RGB to luminance
float rgb2lum(vec3 rgb)
{
    return dot(rgb, LUM);
}

void main()
{
    vec2 invRes = 1.0 / resolution;
    vec2 uv = fragTexCoord;

    // Sample center and neighbors
    float lumenCenter = rgb2lum(texture(texture0, uv).rgb);
    float lumenNW = rgb2lum(texture(texture0, uv + vec2(-1.0, -1.0) * invRes).rgb);
    float lumenNE = rgb2lum(texture(texture0, uv + vec2(1.0, -1.0) * invRes).rgb);
    float lumenSW = rgb2lum(texture(texture0, uv + vec2(-1.0, 1.0) * invRes).rgb);
    float lumenSE = rgb2lum(texture(texture0, uv + vec2(1.0, 1.0) * invRes).rgb);

    // Compute min/max luminance from neighbors
    float lumenMin = min(min(min(lumenNW, lumenNE), lumenSW), lumenSE);
    float lumenMax = max(max(max(lumenNW, lumenNE), lumenSW), lumenSE);

    // Early exit: if contrast is low, no aliasing to fix
    float lumenRange = lumenMax - lumenMin;
    if (lumenRange < max(FXAA_REDUCE_MIN, lumenMax * FXAA_REDUCE_MIN))
    {
        fragColor = texture(texture0, uv);
        return;
    }

    // Compute gradient in each direction
    float lumenNS = lumenNW + lumenNE + lumenCenter * 2.0 + lumenSW + lumenSE;
    float lumenEW = lumenNW + lumenSW + lumenCenter * 2.0 + lumenNE + lumenSE;

    // Determine edge direction (horizontal or vertical)
    float dirX = -abs((lumenNW + lumenNE) - (lumenSW + lumenSE));
    float dirY = -abs((lumenNW + lumenSW) - (lumenNE + lumenSE));

    // Select the dominant edge direction
    vec2 dir;
    float dirReduce;
    if (abs(dirX) > abs(dirY))
    {
        dir = vec2(1.0, 0.0);
        dirReduce = abs((lumenNW + lumenNE) - (lumenSW + lumenSE));
    }
    else
    {
        dir = vec2(0.0, 1.0);
        dirReduce = abs((lumenNW + lumenSW) - (lumenNE + lumenSE));
    }

    // Compute the direction to step along the edge
    float rcpDirMin = 1.0 / (min(abs(dirX), abs(dirY)) + 0.0001);
    vec2 dirNorm = normalize(dir * rcpDirMin);

    // Calculate initial step distance
    float dirMin = min(abs(dirX), abs(dirY));
    float dirMax = max(abs(dirX), abs(dirY));
    float stepDist = min(FXAA_SPAN_MAX, max(dirReduce * FXAA_REDUCE_MUL, FXAA_REDUCE_MIN) / (dirMin + 0.0001));

    // Sample along the edge in both directions
    vec3 colorA = vec3(0.0);
    vec3 colorB = vec3(0.0);
    float distA = 0.0;
    float distB = 0.0;

    for (int i = 0; i < 2; i++)
    {
        float sign = (i == 0) ? -1.0 : 1.0;
        vec2 offset = dirNorm * sign * stepDist * invRes;

        vec2 uv1 = uv + offset;
        vec2 uv2 = uv + offset * 2.0;

        float lumen1 = rgb2lum(texture(texture0, uv1).rgb);
        float lumen2 = rgb2lum(texture(texture0, uv2).rgb);

        float avgLumen = (lumen1 + lumen2) * 0.5;

        if (i == 0)
        {
            colorA = (texture(texture0, uv1).rgb + texture(texture0, uv2).rgb) * 0.5;
            distA = stepDist;
        }
        else
        {
            colorB = (texture(texture0, uv1).rgb + texture(texture0, uv2).rgb) * 0.5;
            distB = stepDist;
        }
    }

    // Blend between the two edge samples
    float distRatio = distA / (distA + distB + 0.0001);
    vec3 color = mix(colorA, colorB, distRatio);

    // Also blend with center pixel for smoother result
    vec3 centerColor = texture(texture0, uv).rgb;
    color = mix(color, centerColor, 0.5);

    fragColor = vec4(color, 1.0);
}
