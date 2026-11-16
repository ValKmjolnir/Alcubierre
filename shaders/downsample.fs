#version 330

// Box downsample from the SSAA scene texture to native resolution.
// Four bilinear taps at the quadrant centres of each output pixel's source footprint.

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

void main()
{
    vec2 sourceSize = vec2(textureSize(texture0, 0));
    vec2 ratio = sourceSize / resolution;
    vec2 offset = 0.25 * ratio / sourceSize;

    vec4 color = texture(texture0, fragTexCoord + vec2(-offset.x, -offset.y))
               + texture(texture0, fragTexCoord + vec2( offset.x, -offset.y))
               + texture(texture0, fragTexCoord + vec2(-offset.x,  offset.y))
               + texture(texture0, fragTexCoord + vec2( offset.x,  offset.y));

    fragColor = color * 0.25;
}
