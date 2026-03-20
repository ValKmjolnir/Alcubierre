#version 330

// Vertical Gaussian blur
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 texelSize;  // (0.0, 1.0/height)
uniform float blurRadius;

void main()
{
    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    // Gaussian blur kernel
    int samples = int(blurRadius);

    for (int i = -8; i <= 8; i++)
    {
        if (i > samples) break;

        float weight = exp(-0.5 * float(i * i) / (float(samples) * float(samples)) * 9.0);
        vec2 offset = vec2(0.0, float(i) * texelSize.y);
        color += texture(texture0, fragTexCoord + offset).rgb * weight;
        totalWeight += weight;
    }

    fragColor = vec4(color / totalWeight, 1.0);
}
