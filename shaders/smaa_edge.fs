#version 330

// SMAA (Subpixel Morphological Anti-Aliasing) - Fragment Shader
// Pass 1: Edge Detection
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

// Luminance weights (ITU-R BT.709)
const vec3 LUM = vec3(0.2126, 0.7152, 0.0722);

// Lower threshold = more edges detected
const float SMAA_THRESHOLD = 0.05;

float rgb2lum(vec3 rgb)
{
    return dot(rgb, LUM);
}

void main()
{
    vec2 texelSize = 1.0 / resolution;
    vec2 uv = fragTexCoord;

    // Sample luminance in cross pattern
    float lumC = rgb2lum(texture(texture0, uv).rgb);
    float lumN = rgb2lum(texture(texture0, uv + vec2(0.0, -texelSize.y)).rgb);
    float lumS = rgb2lum(texture(texture0, uv + vec2(0.0,  texelSize.y)).rgb);
    float lumE = rgb2lum(texture(texture0, uv + vec2( texelSize.x, 0.0)).rgb);
    float lumW = rgb2lum(texture(texture0, uv + vec2(-texelSize.x, 0.0)).rgb);

    // Simple luminance gradient edge detection
    float edgeH = abs(lumW - lumE);  // Horizontal edge (vertical gradient)
    float edgeV = abs(lumN - lumS);  // Vertical edge (horizontal gradient)

    // Apply threshold (soft thresholding)
    edgeH = max(0.0, edgeH - SMAA_THRESHOLD);
    edgeV = max(0.0, edgeV - SMAA_THRESHOLD);

    // Scale to [0, 1] range
    // After threshold subtraction, values are typically in [0, 0.5] range
    edgeH = min(1.0, edgeH * 4.0);
    edgeV = min(1.0, edgeV * 4.0);

    // Encode: R = horizontal edge strength, G = vertical edge strength
    fragColor = vec4(edgeH, edgeV, 0.0, 1.0);
}
