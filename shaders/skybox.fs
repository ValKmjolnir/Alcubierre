#version 330

// Input from vertex shader
in vec3 fragPosition;

// Uniforms
// Note: skybox texture is not currently used; sky is procedurally generated
uniform samplerCube skybox;
uniform float seed;  // Random seed from main program

// Output color
out vec4 fragColor;

// 3D hash function - returns a random value for a 3D grid cell
float hash3(vec3 p, float seed) {
    vec3 p3 = fract(p * vec3(0.1031, 0.1030, 0.0973) + seed * 0.318);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Returns random offset within a 3D grid cell
vec3 hashOffset3(vec3 cell, float seed) {
    return vec3(
        hash3(cell, seed * 1.0) - 0.5,
        hash3(cell, seed * 2.0) - 0.5,
        hash3(cell, seed * 3.0) - 0.5
    );
}

// Generate stars in 3D space - no UV distortion
// dir: normalized view direction
// gridScale: size of each grid cell
// starSize: angular size of each star (smaller = smaller stars)
float stars3D(vec3 dir, float gridScale, float starSize, float seed) {
    // Scale direction to get grid cell
    vec3 p = dir * gridScale;
    vec3 cell = floor(p);
    vec3 localPos = fract(p) - 0.5;
    
    // Get star offset within this cell
    vec3 starOffset = hashOffset3(cell, seed);
    
    // Distance from this fragment to the star
    float dist = length(localPos - starOffset);
    
    // Star with smooth falloff - small, tight disk
    return smoothstep(starSize, 0.0, dist);
}

// Check neighboring cells
float stars3DWithNeighbors(vec3 dir, float gridScale, float starSize, float seed) {
    vec3 warpedDir = normalize(dir + 0.1 * sin(dir * 3.0 + seed));

    vec3 p = warpedDir * gridScale;
    vec3 cell = floor(p);
    vec3 localPos = fract(p);

    float maxBright = 0.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                vec3 neighbor = vec3(float(x), float(y), float(z));
                vec3 testCell = cell + neighbor;

                vec3 starOffset = hashOffset3(testCell, seed + dot(testCell, vec3(0.5)));

                vec3 starWorldPos = testCell + starOffset;
                float dist = length(p - starWorldPos);

                float bright = smoothstep(starSize, 0.0, dist);
                maxBright = max(maxBright, bright);
            }
        }
    }

    return maxBright;
}

void main() {
    vec3 dir = normalize(fragPosition);
    vec3 color = vec3(0.0, 0.0, 0.1);

    // Add starfield using 3D grid - no UV distortion anywhere
    // Each star is a small bright disk with smooth falloff
    
    // Layer 1: Bright stars (sparse, large grid cells)
    float stars1 = stars3DWithNeighbors(dir, 80.0, 0.15, seed * 1.1);
    
    // Layer 2: Medium stars
    float stars2 = stars3DWithNeighbors(dir, 50.0, 0.12, seed * 2.7);
    
    // Layer 3: Dim stars (more numerous)
    float stars3 = stars3DWithNeighbors(dir, 30.0, 0.1, seed * 4.3);
    
    // Star color variation based on direction
    float colorRand = hash3(floor(dir * 100.0), seed * 7.0);
    vec3 starColor;
    if (colorRand < 0.5) {
        starColor = vec3(1.0, 1.0, 1.0);    // White
    } else if (colorRand < 0.65) {
        starColor = vec3(0.75, 0.85, 1.0);  // Blue-white
    } else if (colorRand < 0.8) {
        starColor = vec3(1.0, 0.95, 0.8);   // Yellow-ish
    } else if (colorRand < 0.9) {
        starColor = vec3(1.0, 0.8, 0.6);    // Orange
    } else {
        starColor = vec3(0.9, 0.7, 0.5);    // Red-orange
    }
    
    // Combine layers with different weights
    float totalStars = stars1 + stars2 * 0.7 + stars3 * 0.45;
    
    // Add soft glow for bright stars
    float glow = stars1 * stars1;  // Squared for tighter bright core
    glow = pow(glow, 0.4) * 0.4;
    
    vec3 finalStarColor = (totalStars + glow) * starColor;
    
    // Additive blending
    color += finalStarColor;

    fragColor = vec4(color, 1.0);
}
