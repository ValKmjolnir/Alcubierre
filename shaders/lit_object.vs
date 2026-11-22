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
out vec2 fragTexCoord;

void main()
{
    // Transform position to world space
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;

    // Pass normal in world space (inverse-transpose handles non-uniform scale)
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;
    fragTexCoord = vertexTexCoord;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
