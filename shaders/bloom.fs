#version 330

// Fragment shader for Bloom composite
// This shader is used for additive blending of bloom texture
// The bloom texture is already blurred, we just add it to the scene
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform float bloomIntensity;

void main()
{
    vec3 bloomColor = texture(texture0, fragTexCoord).rgb;
    
    // Apply bloom intensity
    vec3 finalColor = bloomColor * bloomIntensity;
    
    fragColor = vec4(finalColor, 1.0);
}
