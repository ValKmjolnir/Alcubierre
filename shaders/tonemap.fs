#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float exposure;

out vec4 fragColor;

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb * exposure;
    color = color * (1.0 + color / (exposure * exposure)) / (1.0 + color);
    fragColor = vec4(color, 1.0);
}
