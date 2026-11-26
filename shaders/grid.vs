#version 330

// Circular tactical grid: pass the local XZ position to the fragment stage so
// the rings and spokes can be measured in world units.

in vec3 vertexPosition;

uniform mat4 mvp;

out vec2 fragLocal;

void main() {
    fragLocal = vertexPosition.xz;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
