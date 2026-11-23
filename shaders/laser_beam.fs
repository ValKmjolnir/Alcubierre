#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec4 color;
uniform float intensity;
uniform float pulse;

out vec4 fragColor;

void main()
{
    // The whole beam lights up and dims together; pulse is the temporal
    // brightness cycle, there is no spatial sweep along the line.
    float brightness = color.a * intensity * pulse;
    vec3 core = mix(color.rgb, vec3(1.0), 0.3);

    fragColor = vec4(core * brightness, 1.0);
}
