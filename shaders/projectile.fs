#version 330

// Input fragment attributes
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

// Input uniform attributes
uniform vec4 color;         // Projectile color (rgb) and base alpha (a)
uniform float intensity;    // Glow intensity multiplier
uniform float glowRadius;   // Glow radius multiplier
uniform vec3 projPosition;  // Projectile position (world space)
uniform vec3 projVelocity;  // Projectile velocity (world space)
uniform float projAge;      // Projectile age
uniform float projLifetime; // Projectile lifetime

// Output fragment color
out vec4 fragColor;

// Calculate distance from point to line segment (for trail effect)
float distanceToTrail(vec3 point, vec3 start, vec3 end)
{
    vec3 lineVec = end - start;
    vec3 pointVec = point - start;
    float lineLen = length(lineVec);

    if (lineLen < 0.0001) {
        return length(pointVec);
    }

    float t = clamp(dot(pointVec, lineVec) / (lineLen * lineLen), 0.0, 1.0);
    vec3 closestPoint = start + t * lineVec;

    return length(point - closestPoint);
}

void main()
{
    // Calculate velocity direction for heating effect
    float velocityMagnitude = length(projVelocity);
    vec3 velocityDir = (velocityMagnitude > 0.1) ? normalize(projVelocity) : vec3(0.0, 0.0, -1.0);

    // Calculate distance from projectile center
    vec3 toFrag = fragPosition - projPosition;
    float dist = length(toFrag);

    // Calculate angle between fragment normal and velocity direction
    // This determines how "front-facing" this fragment is
    float velocityDot = dot(normalize(toFrag + 0.001), velocityDir);

    // Core projectile radius
    float coreRadius = 0.5;
    float glowMax = coreRadius * glowRadius;

    // ========== REALISTIC HEATED PROJECTILE EFFECT ==========

    // 1. Base glow falloff - smoother exponential decay
    float glowFactor = 0.0;
    if (dist < coreRadius) {
        glowFactor = 1.0;
    } else if (dist < glowMax) {
        float t = (dist - coreRadius) / (glowMax - coreRadius);
        // Use quadratic curve for more natural falloff
        glowFactor = pow(1.0 - t, 2.0);
    }

    // 2. White-hot core - very bright center simulating extreme heat
    // Keep core small for realistic heated tip effect
    float whiteCoreRadius = coreRadius * 0.15;
    float whiteHotFactor = 0.0;
    if (dist < whiteCoreRadius) {
        // Intense white center with smooth falloff
        whiteHotFactor = pow(1.0 - (dist / whiteCoreRadius), 1.5);
    }

    // 3. Velocity-based heating - front gets hotter due to air friction
    float heatingFactor = 0.0;
    if (velocityMagnitude > 1.0) {
        // Only heat the front hemisphere
        if (velocityDot > 0.0) {
            // Stronger heating at the very front
            heatingFactor = pow(velocityDot, 0.5) * min(velocityMagnitude / 20.0, 1.5);
        }
    }

    // 4. Color grading for realistic heated metal look
    // Hot = white/yellow, cooler = orange/red
    vec3 finalColor = color.rgb / 255.0;

    // Add white-hot core
    if (whiteHotFactor > 0.01) {
        finalColor = mix(finalColor, vec3(1.0, 0.95, 0.85), whiteHotFactor * 0.95);
    }

    // Add velocity heating (yellow-white at front)
    if (heatingFactor > 0.01) {
        vec3 heatedColor = vec3(1.0, 0.9, 0.6); // Warm yellow-white
        finalColor = mix(finalColor, heatedColor, heatingFactor * 0.7);
    }

    // 5. Calculate brightness with multiple factors
    float baseBrightness = glowFactor * intensity;

    // Boost brightness at front due to heating
    float heatingBrightness = 1.0 + heatingFactor * 0.8;
    baseBrightness *= heatingBrightness;

    // Add extra brightness for white-hot core
    baseBrightness += whiteHotFactor * 0.5;

    // 6. Age-based effects
    float ageFactor = 1.0;
    if (projAge > projLifetime * 0.75) {
        float remainingLife = 1.0 - (projAge / projLifetime);
        ageFactor = remainingLife * 1.33;

        // Subtle flicker as projectile loses energy (less cartoonish)
        float flicker = sin(projAge * 30.0) * 0.15 + 0.85;
        baseBrightness *= flicker * ageFactor;
    }

    // 7. Apply final brightness
    finalColor *= baseBrightness;

    // 8. Alpha based on brightness for proper blending
    float alpha = clamp(baseBrightness * color.a, 0.0, 1.0);

    fragColor = vec4(finalColor, alpha);
}
