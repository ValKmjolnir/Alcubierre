#version 330

// Alcubierre Warp Drive - Gravitational Lensing and Bubble Wall Optics
// The bubble wall is fixed at screen center (interior observer perspective).
// Velocity direction only affects Doppler shift and relativistic beaming.

in vec2 fragTexCoord;

uniform sampler2D texture0;       // Input texture (bloomed scene)
uniform vec3 velocity;            // Velocity direction; magnitude = beta = v/c (0~0.99)
uniform float warpFactor;         // Warp factor (1.0 = stationary, >1 = superluminal)
uniform float bubbleRadius;       // Warp bubble radius (screen space, 0.3~0.8)
uniform float wallThickness;      // Bubble wall thickness
uniform float exposure;           // Exposure

out vec4 fragColor;

// ============================================================================
// Core optical effects
// ============================================================================

// Bubble wall refraction — gravitational lensing distortion.
// The bubble wall is fixed at screen center and does not follow velocity.
vec2 applyBubbleRefraction(vec2 uv, float warpF) {
    vec2 center = vec2(0.5);
    vec2 delta = uv - center;
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

// Doppler shift — depends on velocity direction.
// `beta` comes from the velocity vector magnitude (v/c).
vec3 applyDopplerShift(vec3 color, vec2 uv, vec3 velDir, float beta) {
    // Screen center = forward direction; edges = lateral / aft
    vec2 fromCenter = uv - 0.5;
    float distFromCenter = length(fromCenter);

    // Projected velocity direction on screen
    vec3 normVel = normalize(velDir + 0.001);
    vec2 velProj = normalize(normVel.xy + 0.001);

    // Cosine between view direction and velocity direction
    float cosTheta;
    if (distFromCenter > 0.001) {
        vec2 viewProj = normalize(fromCenter);
        cosTheta = dot(viewProj, velProj);
    } else {
        cosTheta = 1.0; // dead ahead
    }

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

// Relativistic beaming — light converges toward velocity direction.
// `beta` comes from the velocity vector magnitude (v/c).
float applyBeaming(vec2 uv, vec3 velDir, float beta) {
    vec2 fromCenter = uv - 0.5;
    float dist = length(fromCenter);
    if (dist < 0.001) return 1.0 / pow(max(0.001, 1.0 - beta), 2.0);

    vec3 normVel = normalize(velDir + 0.001);
    vec2 velProj = normalize(normVel.xy + 0.001);
    vec2 viewProj = normalize(fromCenter);

    float cosTheta = dot(viewProj, velProj);
    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));
    float doppler = 1.0 / (gamma * (1.0 - beta * cosTheta + 0.001));

    return pow(max(doppler, 0.0), 2.5);
}

// Forward glow — bright spot fixed at screen center.
vec3 forwardGlow(vec2 uv, float warpF) {
    float dist = length(uv - 0.5);
    float glowRadius = 0.25 / max(warpF, 2.0);
    float glow = exp(-dist * dist / (glowRadius * glowRadius));

    return vec3(0.4, 0.6, 1.0) * glow * (warpF - 1.0) * 0.6;
}

// Bow wave radiation — expands outward from center.
vec3 bowWave(vec2 uv, float warpF) {
    vec2 fromCenter = uv - 0.5;
    float dist = length(fromCenter);
    if (dist < 0.001) return vec3(0.0);

    vec2 dir = fromCenter / dist;

    // Annular radiation, stronger in the forward direction
    float forwardCos = dir.y; // assume upward is forward
    float forwardFactor = max(forwardCos, 0.0);

    float radialFade = exp(-dist * 3.0);
    float coneShape = pow(forwardFactor, 1.5);

    return vec3(0.3, 0.5, 0.9) * coneShape * radialFade * (warpF - 1.0) * 0.25;
}

// Bubble wall annular glow — fixed at screen center.
float bubbleWallGlow(vec2 uv, float warpF) {
    float dist = length(uv - 0.5) * 2.0;
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
    float dist = length(uv - 0.5) * 2.0;
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
    vec2 uv = fragTexCoord;

    // Step 1: Bubble wall refraction (driven by warpFactor)
    vec2 distortedUV = applyBubbleRefraction(uv, warpFactor);
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    // Step 2: Chromatic aberration (driven by warpFactor)
    vec3 color = applyChromaticAberration(distortedUV, warpFactor);

    // Step 3: Doppler shift (driven by beta = v/c)
    color = applyDopplerShift(color, distortedUV, velDir, beta);

    // Step 4: Relativistic beaming (driven by beta = v/c)
    float beam = applyBeaming(distortedUV, velDir, beta);
    color *= beam;

    // Step 5: Forward glow (driven by warpFactor)
    color += forwardGlow(uv, warpFactor);

    // Step 6: Bow wave radiation (driven by warpFactor)
    color += bowWave(uv, warpFactor);

    // Step 7: Bubble wall annular glow (driven by warpFactor)
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
