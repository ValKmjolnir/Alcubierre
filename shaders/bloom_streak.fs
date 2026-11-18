#version 330

// Horizontal anamorphic streak from the flare-source buffer
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 texelSize;   // (1.0/scene width, 0.0)
uniform float stepPx;     // native px between taps
uniform float sigmaTaps;  // gaussian sigma, in tap units
uniform vec3 tint;
uniform float gain;

void main()
{
    const int samples = 24;
    float falloff = 0.5 / (sigmaTaps * sigmaTaps);

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    for (int i = -samples; i <= samples; i++)
    {
        float weight = exp(-float(i * i) * falloff);
        vec2 offset = vec2(float(i) * stepPx * texelSize.x, 0.0);
        color += texture(texture0, fragTexCoord + offset).rgb * weight;
        totalWeight += weight;
    }

    color = color / totalWeight;
    fragColor = vec4(color * tint * gain, 1.0);
}
