#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform attributes
uniform mat4 mvp;
uniform vec3 projPosition;  // Projectile position (world space)
uniform vec3 camRight;      // Camera basis for billboarding
uniform vec3 camUp;
uniform float glowRadius;   // Billboard half size (world units)

out vec2 vLocal;

void main()
{
    vLocal = vertexPosition.xy;

    vec3 world = projPosition
               + (camRight * vertexPosition.x + camUp * vertexPosition.y) * glowRadius;

    gl_Position = mvp * vec4(world, 1.0);
}
