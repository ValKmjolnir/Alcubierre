#version 330

// Extract bright areas from scene
// Only pixels brighter than threshold will be extracted
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform float brightnessThreshold;

void main()
{
    vec3 color = texture(texture0, fragTexCoord).rgb;

    // Calculate luminance
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));

    // Extract only bright pixels
    float brightness = max(0.0, luminance - brightnessThreshold) / (1.0 - brightnessThreshold);

    // Preserve color while extracting brightness
    vec3 brightColor = color * brightness;

    fragColor = vec4(brightColor, 1.0);
}
