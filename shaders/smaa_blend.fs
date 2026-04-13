#version 330

// SMAA (Subpixel Morphological Anti-Aliasing)
// Pass 2: Blend Weight Calculation
in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;  // Edge detection texture from Pass 1
uniform vec2 resolution;

const int SMAA_SEARCH_STEPS = 16;

vec2 texelSize = 1.0 / resolution;

// Search along horizontal edge direction (R channel only)
float searchHorizontal(sampler2D edgeTex, vec2 uv, float dir)
{
    float dist = 0.0;
    for (int i = 0; i < SMAA_SEARCH_STEPS; i++)
    {
        vec2 offset = vec2(dir * (float(i) + 0.5) * texelSize.x, 0.0);
        float edge = texture(edgeTex, uv + offset).r;
        if (edge < 0.02) break;
        dist = float(i) + 1.0;
    }
    return dist;
}

// Search along vertical edge direction (G channel only)
float searchVertical(sampler2D edgeTex, vec2 uv, float dir)
{
    float dist = 0.0;
    for (int i = 0; i < SMAA_SEARCH_STEPS; i++)
    {
        vec2 offset = vec2(0.0, dir * (float(i) + 0.5) * texelSize.y);
        float edge = texture(edgeTex, uv + offset).g;
        if (edge < 0.02) break;
        dist = float(i) + 1.0;
    }
    return dist;
}

void main()
{
    vec2 uv = fragTexCoord;
    vec4 edge = texture(texture0, uv);
    float edgeH = edge.r;
    float edgeV = edge.g;

    if (edgeH < 0.02 && edgeV < 0.02)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Search along edge directions
    float leftH  = 0.0, rightH = 0.0;
    float upV    = 0.0, downV  = 0.0;

    if (edgeH > 0.02)
    {
        leftH  = searchHorizontal(texture0, uv, -1.0);
        rightH = searchHorizontal(texture0, uv,  1.0);
    }
    if (edgeV > 0.02)
    {
        upV   = searchVertical(texture0, uv, -1.0);
        downV = searchVertical(texture0, uv,  1.0);
    }

    // Compute blend weights: position within the edge segment
    // 0.0 = one endpoint, 0.5 = center, 1.0 = other endpoint
    float blendH = 0.0, blendV = 0.0;

    if (edgeH > 0.02)
    {
        float totalH = leftH + rightH + 1.0;
        blendH = (leftH + 0.5) / totalH;
    }
    if (edgeV > 0.02)
    {
        float totalV = upV + downV + 1.0;
        blendV = (upV + 0.5) / totalV;
    }

    // Compute blend strength based on edge length
    float strengthH = edgeH * min(1.0, (leftH + rightH) / float(SMAA_SEARCH_STEPS));
    float strengthV = edgeV * min(1.0, (upV + downV) / float(SMAA_SEARCH_STEPS));

    // R = blend weight for horizontal edge (position along H edge)
    // G = blend weight for vertical edge (position along V edge)
    // B = combined blend strength
    fragColor = vec4(blendH, blendV, max(strengthH, strengthV), 1.0);
}
