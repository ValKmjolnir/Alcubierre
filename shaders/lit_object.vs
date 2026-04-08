#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

// Input uniform attributes
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes
out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
    // Transform position to world space
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;

    // Pass normal in model space (will be normalized in fragment shader)
    fragNormal = vertexNormal;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
