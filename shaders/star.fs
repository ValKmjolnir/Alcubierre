#version 330

// Input fragment attributes
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

// Input uniform attributes
uniform vec4 color;             // Star color (rgb) and base alpha (a)
uniform float intensity;        // Glow intensity multiplier
uniform float glowRadius;       // Glow radius multiplier
uniform vec3 starPosition;      // Star position (world space)

// Output fragment color
out vec4 fragColor;

void main()
{
    // Calculate distance from star center
    vec3 toFrag = fragPosition - starPosition;
    float dist = length(toFrag);

    // Core star radius (from sphere geometry)
    float coreRadius = 0.5;

    // Extended glow radius for stellar corona effect
    float glowMax = coreRadius * glowRadius;

    // ========== STELLAR EMISSIVE LIGHTING EFFECT ==========

    // 1. Base radial falloff - smooth exponential decay
    float glowFactor = 0.0;

    if (dist < coreRadius) {
        // Inside star core - full brightness
        glowFactor = 1.0;
    } else if (dist < glowMax) {
        // In corona/glow region - gradual falloff
        float t = (dist - coreRadius) / (glowMax - coreRadius);
        // Use power curve for realistic stellar atmosphere falloff
        glowFactor = pow(1.0 - t, 1.5);
    } else {
        // Outside glow region
        glowFactor = 0.0;
    }

    // 2. Hot white core simulation
    float whiteCoreRadius = coreRadius * 0.3;
    float whiteHotFactor = 0.0;

    if (dist < whiteCoreRadius) {
        // Intense white-hot center with smooth falloff
        whiteHotFactor = pow(1.0 - (dist / whiteCoreRadius), 2.0);
    }

    // 3. Limb darkening effect (edges appear darker/redder)
    // This simulates the optical depth effect in real stars
    float limbDarkening = 1.0;
    if (dist > coreRadius * 0.7 && dist < coreRadius) {
        float limbT = (dist - coreRadius * 0.7) / (coreRadius * 0.3);
        limbDarkening = 1.0 - limbT * 0.3; // 30% darker at edges
    }

    // 4. Color grading for stellar blackbody radiation
    vec3 finalColor = color.rgb;

    // Add white-hot core
    if (whiteHotFactor > 0.01) {
        // Mix toward white/yellow for hot center
        finalColor = mix(finalColor, vec3(1.0, 0.98, 0.9), whiteHotFactor * 0.9);
    }

    // Apply limb darkening
    finalColor *= limbDarkening;

    // 5. Calculate final brightness with multiple factors
    float baseBrightness = glowFactor * intensity;

    // Add extra luminance for white-hot core
    baseBrightness += whiteHotFactor * 0.8;

    // 6. Apply final color and alpha
    finalColor *= baseBrightness;

    // Alpha based on glow factor for proper additive blending
    float alpha = clamp(glowFactor * color.a, 0.0, 1.0);

    fragColor = vec4(finalColor, alpha);
}
