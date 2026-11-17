#version 330

// Alcubierre Warp Drive - Gravitational Lensing and Bubble Wall Optics
// The bubble wall is fixed at screen center (interior observer perspective).
// Velocity direction only affects Doppler shift and relativistic beaming.

in vec2 fragTexCoord;

uniform sampler2D texture0;       // Input texture (bloomed scene)
uniform vec3 velocity;            // Velocity direction; magnitude = beta = v/c
uniform vec3 viewDirection;       // Camera forward direction (normalized)
uniform float warpFactor;         // Warp factor (1.0 = stationary, >1 = superluminal)
uniform float bubbleRadius;       // Warp bubble radius (screen space, 0.3~0.8)
uniform float wallThickness;      // Bubble wall thickness
uniform float aspectRatio;        // Screen aspect ratio (width / height)
uniform float fovY;               // Camera vertical FOV in degrees

out vec4 fragColor;

// ============================================================================
// Core optical effects
// ============================================================================

// Correct UV delta for screen aspect ratio so that distances are isotropic.
vec2 correctAspect(vec2 delta, float ar) {
    return vec2(delta.x * ar * 0.8, delta.y);
}

vec2 uncorrectAspect(vec2 delta, float ar) {
    return vec2(delta.x / (ar * 0.8), delta.y);
}

// Project a 3D direction onto screen UV, given the camera view direction.
// Returns UV in [0,1] with center at (0.5, 0.5), plus z; z <= 0 means behind camera.
vec3 projectToScreen(vec3 worldDir, vec3 viewDir, float ar) {
    // Build camera basis from viewDirection
    vec3 forward = normalize(viewDir);
    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(forward, worldUp));
    vec3 up = cross(right, forward);

    // Express worldDir in camera local space
    float x = dot(worldDir, right);
    float y = dot(worldDir, up);
    float z = dot(worldDir, forward);

    // Behind camera
    if (z <= 0.0) return vec3(0.0, 0.0, z);

    // Perspective projection, matching the camera's actual vertical FOV
    float focal = 0.5 / tan(radians(0.5 * fovY));
    float screenX = focal / ar * (x / z);
    float screenY = focal * (y / z);

    // Convert to UV [0,1] with center at (0.5, 0.5)
    return vec3(screenX + 0.5, -screenY + 0.5, z);
}

// Bubble wall refraction — gravitational lensing distortion.
// The bubble wall is fixed at screen center and does not follow velocity.
vec2 applyBubbleRefraction(vec2 uv, float warpF) {
    vec2 center = vec2(0.5);
    vec2 delta = correctAspect(uv - center, aspectRatio);
    float dist = length(delta);

    if (dist < 0.001) return uv;

    vec2 radialDir = delta / dist;

    // Annular region of influence centered at bubbleRadius
    float wallInner = max(0.01, bubbleRadius - wallThickness);
    float wallOuter = min(0.99, bubbleRadius + wallThickness);
    float wallCenter = (wallInner + wallOuter) * 0.5;
    float wallSigma = (wallOuter - wallInner) * 0.55;

    float wallFactor = exp(-pow((dist - wallCenter) / wallSigma, 2.0) * 0.5);

    // Einstein deflection angle (analogous to gravitational lensing)
    float einsteinAngle = warpF * 0.1;
    float distortion = einsteinAngle * wallFactor / (dist + 0.02);

    // Radial distortion — light bends toward the bubble wall
    float pullLimit = 0.75 * wallInner;
    float pullScale = min(1.0, pullLimit * (wallCenter + 0.02) / (0.29 * max(einsteinAngle, 1e-4)));
    float pull = distortion * 0.29 * pullScale;
    uv -= uncorrectAspect(radialDir, aspectRatio) * pull;

    // Tangential swirl — produces arc-like distortion
    vec2 tangentDir = vec2(-delta.y, delta.x) / dist;
    float tangentialDistortion = einsteinAngle * wallFactor * 0.12;
    uv += uncorrectAspect(tangentDir, aspectRatio) * tangentialDistortion;

    return uv;
}

// Chromatic aberration — wavelength-dependent refraction index.
vec3 applyChromaticAberration(vec2 uv, float warpF) {
    float distortion = min(warpF * 0.003, 0.005);

    float r = texture(texture0, uv + vec2(distortion, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(distortion, 0.0)).b;

    return vec3(r, g, b);
}

// Forward glow — bright spot at the projected velocity direction on screen.
vec3 forwardGlow(vec2 uv, vec3 velDir, vec3 viewDir, float warpF) {
    vec3 velPos = projectToScreen(velDir, viewDir, aspectRatio);
    if (velPos.z <= 0.0) return vec3(0.0); // behind camera
    vec2 velScreenPos = velPos.xy;

    vec2 fromPos = correctAspect(uv - velScreenPos, aspectRatio);
    float dist = length(fromPos);
    float glowRadius = 0.25 / max(warpF, 2.0);
    float glow = exp(-dist * dist / (glowRadius * glowRadius));

    return vec3(0.4, 0.6, 1.0) * glow * (warpF - 1.0) * 0.6;
}

// Bow wave radiation — compressed spacetime ahead of the ship.
// Positioned at the screen-projected velocity direction.
vec3 bowWave(vec2 uv, vec3 velDir, vec3 viewDir, float warpF) {
    // Where does the velocity direction land on screen?
    vec3 velPos = projectToScreen(velDir, viewDir, aspectRatio);
    if (velPos.z <= 0.0) return vec3(0.0); // behind camera
    vec2 velScreenPos = velPos.xy;

    // Aspect-corrected distance from bow wave center
    vec2 fromCenter = correctAspect(uv - velScreenPos, aspectRatio);
    float dist = length(fromCenter);
    if (dist < 0.001) return vec3(0.0);

    // Radial ring glow — peaks at mid distance
    float radialGlow = exp(-dist * 2.5) * dist * 2.72;  // peaks near dist~0.4

    return vec3(0.3, 0.5, 0.9) * radialGlow * (warpF - 1.0) * 0.3;
}

// Bubble wall annular glow — fixed at screen center.
float bubbleWallGlow(vec2 uv, float warpF) {
    float dist = length(correctAspect(uv - 0.5, aspectRatio));
    float wallInner = max(0.01, bubbleRadius - wallThickness);
    float wallOuter = min(0.99, bubbleRadius + wallThickness);

    // Gaussian distribution along the ring
    float wallCenter = (wallInner + wallOuter) * 0.5;
    float wallSigma = (wallOuter - wallInner) * 0.5 + 0.02;
    float glow = exp(-pow(dist - wallCenter, 2.0) / (2.0 * wallSigma * wallSigma));

    return glow * warpF * 0.25;
}

// Bubble interior — flat spacetime region.
float bubbleInterior(vec2 uv) {
    float dist = length(correctAspect(uv - 0.5, aspectRatio));
    float interiorEdge = max(0.01, bubbleRadius - wallThickness - 0.03);
    return 1.0 - smoothstep(interiorEdge - 0.05, interiorEdge, dist);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    float beta = clamp(length(velocity), 0.0, 0.99);

    // Passthrough for sub-relativistic speeds
    if (beta < 0.01 && warpFactor < 1.05) {
        fragColor = texture(texture0, fragTexCoord);
        return;
    }

    vec3 velDir = normalize(velocity + 0.0001);
    vec3 viewDir = normalize(viewDirection + 0.0001);
    vec2 uv = fragTexCoord;

    // Step 1: Bubble wall refraction (driven by warpFactor)
    vec2 distortedUV = applyBubbleRefraction(uv, warpFactor);
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    // Step 2: Chromatic aberration (driven by warpFactor)
    vec3 color = applyChromaticAberration(distortedUV, warpFactor);

    // Step 3: Forward glow (positioned at projected velocity on screen)
    float betaFade = smoothstep(0.0, 0.15, beta);
    color += forwardGlow(uv, velDir, viewDir, warpFactor) * betaFade;

    // Step 4: Bow wave radiation (positioned at projected velocity on screen)
    color += bowWave(uv, velDir, viewDir, warpFactor) * betaFade;

    // Step 5: Bubble wall annular glow (driven by warpFactor, centered on screen)
    float wallGlow = bubbleWallGlow(uv, warpFactor);
    color += vec3(0.3, 0.5, 1.0) * wallGlow;

    // Step 6: Slight interior brightening (flat spacetime region)
    float interior = bubbleInterior(uv);
    color = mix(color, color * 1.1, interior * 0.3);

    fragColor = vec4(color, 1.0);
}
