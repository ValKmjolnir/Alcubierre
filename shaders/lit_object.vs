#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

// Input uniform attributes
uniform mat4 mvp;
uniform mat4 matModel;       // Model matrix (provided by raylib)
uniform mat3 matNormalMatrix; // Normal matrix (provided by raylib)

// Output vertex attributes
out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
    // Transform position to world space
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));

    // Transform normal to world space
    fragNormal = normalize(matNormalMatrix * vertexNormal);

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
