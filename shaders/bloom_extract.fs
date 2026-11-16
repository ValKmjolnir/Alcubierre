#version 330

// Extract bright areas from scene
// Only pixels brighter than threshold will be extracted
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform float brightnessThreshold;

const vec3 LUM = vec3(0.299, 0.587, 0.114);

void main()
{
    vec2 offset = 0.5 / vec2(textureSize(texture0, 0));

    vec3 c0 = texture(texture0, fragTexCoord + vec2(-offset.x, -offset.y)).rgb;
    vec3 c1 = texture(texture0, fragTexCoord + vec2( offset.x, -offset.y)).rgb;
    vec3 c2 = texture(texture0, fragTexCoord + vec2(-offset.x,  offset.y)).rgb;
    vec3 c3 = texture(texture0, fragTexCoord + vec2( offset.x,  offset.y)).rgb;

    vec3 color = (c0 + c1 + c2 + c3) * 0.25;

    float lumaMax = max(max(dot(c0, LUM), dot(c1, LUM)), max(dot(c2, LUM), dot(c3, LUM)));

    float knee = min(brightnessThreshold * 1.3, 1.0);
    float brightness = smoothstep(brightnessThreshold, knee, lumaMax);

    fragColor = vec4(color * brightness, 1.0);
}
