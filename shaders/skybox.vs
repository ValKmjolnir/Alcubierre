#version 330

// Vertex attributes
in vec3 vertexPosition;

// Uniforms
uniform mat4 mvp;

// Output to fragment shader
out vec3 fragPosition;

void main() {
    // Transform vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    
    // Pass position to fragment shader (for skybox, we use the view direction)
    fragPosition = vertexPosition;
}
