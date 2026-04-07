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
uniform float exposure;           // Exposure

out vec4 fragColor;

// ============================================================================
// Core optical effects
// ============================================================================

// Correct UV delta for screen aspect ratio so that distances are isotropic.
vec2 correctAspect(vec2 delta, float ar) {
    return vec2(delta.x * ar * 0.8, delta.y);
}

// Project a 3D direction onto screen UV, given the camera view direction.
// Returns UV in [0,1] with center at (0.5, 0.5). Returns (-1,-1) if behind camera.
vec2 projectToScreen(vec3 worldDir, vec3 viewDir, float ar) {
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
    if (z <= 0.0) return vec2(-1.0);

    // Perspective projection
    float fovScale = 1.0;  // matches the ~45 deg FOV
    vec2 ndc = vec2(x, y) / (z * fovScale);

    // Convert to UV [0,1] with aspect correction
    return vec2(ndc.x / ar * 0.8 + 0.5, -ndc.y + 0.5);
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

    // Strongest within the annular band
    float wallFactor = smoothstep(wallInner - 0.08, wallInner, dist) *
                       (1.0 - smoothstep(wallOuter, wallOuter + 0.08, dist));

    // Einstein deflection angle (analogous to gravitational lensing)
    float einsteinAngle = warpF * 0.1;
    float distortion = einsteinAngle * wallFactor / (dist + 0.02);

    // Radial distortion — light bends toward the bubble wall
    uv -= radialDir * distortion * 0.5;

    // Tangential swirl — produces arc-like distortion
    vec2 tangentDir = vec2(-delta.y, delta.x) / dist;
    float tangentialDistortion = einsteinAngle * wallFactor * 0.12;
    uv += tangentDir * tangentialDistortion;

    return uv;
}

// Chromatic aberration — wavelength-dependent refraction index.
vec3 applyChromaticAberration(vec2 uv, float warpF) {
    float distortion = warpF * 0.006;

    float r = texture(texture0, uv + vec2(distortion, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(distortion, 0.0)).b;

    return vec3(r, g, b);
}

// Doppler shift — depends on angle between view direction and velocity.
// `beta` comes from the velocity vector magnitude (v/c).
vec3 applyDopplerShift(vec3 color, vec2 uv, vec3 velDir, vec3 viewDir, float beta) {
    // Cosine between view direction and velocity direction
    float cosTheta = dot(normalize(viewDir + 0.001), normalize(velDir + 0.001));

    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));

    // Doppler factor
    float doppler = 1.0 / (gamma * (1.0 - beta * cosTheta + 0.001));

    // Color temperature mapping
    vec3 shifted;
    if (doppler > 1.0) {
        // Blueshift: enhance blue channel
        shifted = vec3(
            color.r * 0.8,
            color.g * doppler * 0.9,
            color.b * doppler * 1.1
        );
    } else {
        // Redshift: enhance red channel
        float redShift = 1.0 / max(0.1, doppler);
        shifted = vec3(
            color.r * redShift * 1.0,
            color.g * doppler * 0.85,
            color.b * doppler * 0.6
        );
    }

    return clamp(shifted, 0.0, 5.0);
}

// Relativistic beaming — brightness boost when looking along velocity.
// `beta` comes from the velocity vector magnitude (v/c).
float applyBeaming(vec3 velDir, vec3 viewDir, float beta) {
    float cosTheta = dot(normalize(viewDir + 0.001), normalize(velDir + 0.001));
    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));
    float doppler = 1.0 / (gamma * (1.0 - beta * cosTheta + 0.001));

    return pow(max(doppler, 0.0), 2.5);
}

// Forward glow — bright spot at the projected velocity direction on screen.
vec3 forwardGlow(vec2 uv, vec3 velDir, vec3 viewDir, float warpF) {
    vec2 velScreenPos = projectToScreen(velDir, viewDir, aspectRatio);
    if (velScreenPos.x < -0.5) return vec3(0.0); // behind camera

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
    vec2 velScreenPos = projectToScreen(velDir, viewDir, aspectRatio);
    if (velScreenPos.x < -0.5) return vec3(0.0); // behind camera

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
    float dist = length(correctAspect(uv - 0.5, aspectRatio)) * 2.0;
    float wallInner = max(0.01, bubbleRadius - wallThickness);
    float wallOuter = min(0.99, bubbleRadius + wallThickness);

    // Gaussian distribution along the ring
    float wallCenter = (wallInner + wallOuter) * 0.5;
    float wallSigma = (wallOuter - wallInner) * 0.5 + 0.02;
    float glow = exp(-pow(dist - wallCenter, 2.0) / (2.0 * wallSigma * wallSigma));

    return glow * warpF * 0.35;
}

// Bubble interior — flat spacetime region.
float bubbleInterior(vec2 uv) {
    float dist = length(correctAspect(uv - 0.5, aspectRatio)) * 2.0;
    float interiorEdge = max(0.01, bubbleRadius - wallThickness);
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

    // Step 3: Doppler shift (driven by viewDir vs velDir angle)
    color = applyDopplerShift(color, distortedUV, velDir, viewDir, beta);

    // Step 4: Relativistic beaming (driven by viewDir vs velDir angle)
    float beam = applyBeaming(velDir, viewDir, beta);
    color *= beam;

    // Step 5: Forward glow (positioned at projected velocity on screen)
    color += forwardGlow(uv, velDir, viewDir, warpFactor);

    // Step 6: Bow wave radiation (positioned at projected velocity on screen)
    color += bowWave(uv, velDir, viewDir, warpFactor);

    // Step 7: Bubble wall annular glow (driven by warpFactor, centered on screen)
    float wallGlow = bubbleWallGlow(uv, warpFactor);
    color += vec3(0.3, 0.5, 1.0) * wallGlow;

    // Step 8: Slight interior brightening (flat spacetime region)
    float interior = bubbleInterior(uv);
    color = mix(color, color * 1.1, interior * 0.3);

    // Step 9: Tone mapping + exposure
    color = color / (1.0 + color);  // Reinhard
    color *= exposure;

    fragColor = vec4(color, 1.0);
}
