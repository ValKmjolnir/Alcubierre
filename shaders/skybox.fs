#version 330

// Input from vertex shader
in vec3 fragPosition;

// Uniforms
uniform samplerCube skybox;

// Output color
out vec4 fragColor;

void main() {
    vec3 dir = normalize(fragPosition);

    // Gradient sky (no texture needed)
    float y = dir.y;

    // Bright gradient colors for better visibility
    vec3 gradientTop = vec3(0.1, 0.3, 0.7);         // Bright blue at top
    vec3 gradientMiddle = vec3(0.3, 0.2, 0.5);      // Purple-ish in middle
    vec3 gradientBottom = vec3(0.5, 0.3, 0.4);      // Light purplish at bottom

    // Smooth interpolation
    float t = y * 0.5 + 0.5;  // Map y from [-1,1] to [0,1]
    vec3 color;
    if (t < 0.5) {
        color = mix(gradientBottom, gradientMiddle, t * 2.0);
    } else {
        color = mix(gradientMiddle, gradientTop, (t - 0.5) * 2.0);
    }

    fragColor = vec4(color, 1.0);
}
