#version 330

in vec3 fragPosition;

uniform samplerCube skybox;

out vec4 fragColor;

void main() {
    fragColor = texture(skybox, normalize(fragPosition));
}
