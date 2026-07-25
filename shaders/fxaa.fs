#version 330

// FXAA 3.11 - Fragment Shader
// Based on NVIDIA FXAA 3.11 by Timothy Lottes
// Ported from McNopper/OpenGL Example42
// Reference: http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

// Quality parameters
const float fxaaQualitySubpix = 0.75;
const float fxaaQualityEdgeThreshold = 0.166;
const float fxaaQualityEdgeThresholdMin = 0.0833;

// Luminance weights (ITU-R BT.709 — consistent with SMAA)
const vec3 LUM = vec3(0.2126, 0.7152, 0.0722);

void main()
{
    vec2 texelStep = 1.0 / resolution;
    vec2 uv = fragTexCoord;

    vec3 rgbM = texture(texture0, uv).rgb;

    // Sample neighbours (using textureOffset which is available in GLSL 330)
    vec3 rgbNW = textureOffset(texture0, uv, ivec2(-1,  1)).rgb;
    vec3 rgbNE = textureOffset(texture0, uv, ivec2( 1,  1)).rgb;
    vec3 rgbSW = textureOffset(texture0, uv, ivec2(-1, -1)).rgb;
    vec3 rgbSE = textureOffset(texture0, uv, ivec2( 1, -1)).rgb;

    // Convert to luma
    float lumaNW = dot(rgbNW, LUM);
    float lumaNE = dot(rgbNE, LUM);
    float lumaSW = dot(rgbSW, LUM);
    float lumaSE = dot(rgbSE, LUM);
    float lumaM  = dot(rgbM,  LUM);

    // Gather min and max luma
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    // Early exit if contrast is below threshold
    if (lumaMax - lumaMin <= lumaMax * fxaaQualityEdgeThreshold)
    {
        fragColor = vec4(rgbM, 1.0);
        return;
    }

    // Sampling direction along the gradient
    vec2 samplingDirection;
    samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    samplingDirection.y =   (lumaNW + lumaSW) - (lumaNE + lumaSE);

    // Sampling step distance depends on luma: brighter areas get less blur
    float samplingDirectionReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * fxaaQualitySubpix,
        fxaaQualityEdgeThresholdMin);

    // Factor for normalising the sampling direction
    float minSamplingDirectionFactor = 1.0 / (
        min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

    // Calculate final sampling direction, clamped and scaled to texel size
    samplingDirection = clamp(
        samplingDirection * minSamplingDirectionFactor,
        vec2(-8.0), vec2(8.0)) * texelStep;

    // Inner samples on the tab
    vec3 rgbSampleNeg = texture(texture0, uv + samplingDirection * (1.0/3.0 - 0.5)).rgb;
    vec3 rgbSamplePos = texture(texture0, uv + samplingDirection * (2.0/3.0 - 0.5)).rgb;
    vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;

    // Outer samples on the tab
    vec3 rgbSampleNegOuter = texture(texture0, uv + samplingDirection * (0.0/3.0 - 0.5)).rgb;
    vec3 rgbSamplePosOuter = texture(texture0, uv + samplingDirection * (3.0/3.0 - 0.5)).rgb;
    vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;

    // Calculate luma of the 4-tab result
    float lumaFourTab = dot(rgbFourTab, LUM);

    // Check if outer samples are beyond the edge
    if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
    {
        // Outer samples exceeded edge range — use only 2 inner samples
        fragColor = vec4(rgbTwoTab, 1.0);
    }
    else
    {
        // Use all 4 samples
        fragColor = vec4(rgbFourTab, 1.0);
    }
}
