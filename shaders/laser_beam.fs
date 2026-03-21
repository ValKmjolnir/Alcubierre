#version 330

// Input fragment attributes
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

// Input uniform attributes
uniform vec4 color;         // Laser color (rgb) and base alpha (a)
uniform float intensity;    // Glow intensity multiplier
uniform float glowRadius;   // Glow radius multiplier
uniform vec3 lineStart;     // Laser line start position (world space)
uniform vec3 lineEnd;       // Laser line end position (world space)

// Output fragment color
out vec4 fragColor;

// Calculate distance from point to line segment
float pointToLineDistance(vec3 point, vec3 lineStart, vec3 lineEnd)
{
    vec3 lineVec = lineEnd - lineStart;
    vec3 pointVec = point - lineStart;
    float lineLen = length(lineVec);
    
    if (lineLen < 0.0001) {
        return length(pointVec);
    }
    
    // Project point onto line, clamp to segment
    float t = clamp(dot(pointVec, lineVec) / (lineLen * lineLen), 0.0, 1.0);
    vec3 closestPoint = lineStart + t * lineVec;
    
    return length(point - closestPoint);
}

void main()
{
    // Calculate distance from laser center line
    float dist = pointToLineDistance(fragPosition, lineStart, lineEnd);
    
    // Core beam radius (based on cylinder radius from texCoord)
    float coreRadius = 0.5;
    
    // Glow radius - make it wider for more visible glow
    float glowMax = coreRadius * glowRadius;
    
    // Calculate glow falloff with exponential curve for brighter appearance
    float glowFactor = 0.0;
    
    if (dist < coreRadius) {
        // Inside core beam - full brightness with boost
        glowFactor = 1.0;
    } else if (dist < glowMax) {
        // In glow region - slower falloff for brighter glow
        float t = (dist - coreRadius) / (glowMax - coreRadius);
        // Use smoother curve that stays brighter longer
        glowFactor = pow(1.0 - t, 0.5);
    }
    
    // Apply intensity boost and color
    float brightness = color.a * glowFactor * intensity;

    // Add white-hot core for extra brightness - make it wider
    vec3 finalColor = color.rgb;
    float whiteCoreRadius = coreRadius * 0.5;
    if (dist < whiteCoreRadius) {
        // White hot center - stronger mix
        float whiteFactor = 1.0 - (dist / whiteCoreRadius);
        finalColor = mix(finalColor, vec3(1.0), whiteFactor * 0.8);
    }

    // Boost color brightness
    finalColor *= brightness;
    
    fragColor = vec4(finalColor, brightness);
}
