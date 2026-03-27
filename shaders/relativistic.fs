#version 330

// Input from vertex shader
in vec2 fragTexCoord;

// Uniforms
uniform sampler2D texture0;
uniform vec3 velocity;
uniform float exposure;

// Output color
out vec4 fragColor;

// ============================================================================
// Relativistic Visual Effects
// ============================================================================

// Lorentz factor: gamma = 1 / sqrt(1 - beta^2)
float lorentzFactor(float beta) {
    return 1.0 / sqrt(max(0.0001, 1.0 - beta * beta));
}

// Relativistic Doppler factor
// f_observed = f_source * dopplerFactor
float dopplerFactor(float cosTheta, float beta) {
    float gamma = lorentzFactor(beta);
    float denom = 1.0 - beta * cosTheta;
    if (abs(denom) < 0.0001) denom = 0.0001;
    return 1.0 / (gamma * denom);
}

// Convert Doppler factor to RGB color shift
vec3 applyDopplerShift(vec3 color, float doppler) {
    doppler = clamp(doppler, 0.05, 20.0);
    float logDoppler = log(doppler);
    
    vec3 shifted;
    shifted.r = color.r * exp(-logDoppler * 0.8);
    shifted.g = color.g * exp(-logDoppler * 0.3);
    shifted.b = color.b * exp(-logDoppler * (-0.6));
    
    return shifted;
}

// Relativistic beaming (headlight effect)
float applyBeaming(float cosTheta, float beta) {
    if (beta < 0.001) return 1.0;
    float doppler = dopplerFactor(cosTheta, beta);
    return pow(doppler, 3.0);
}

// Inverse relativistic aberration
vec3 inverseAberration(vec3 observedDir, vec3 velDir, float beta) {
    if (beta < 0.001) return observedDir;
    
    float cosThetaObs = dot(observedDir, velDir);
    float cosThetaOrig = (cosThetaObs + beta) / (1.0 + beta * cosThetaObs);
    cosThetaOrig = clamp(cosThetaOrig, -1.0, 1.0);
    
    float sinThetaObs = sqrt(max(0.0, 1.0 - cosThetaObs * cosThetaObs));
    float sinThetaOrig = sqrt(max(0.0, 1.0 - cosThetaOrig * cosThetaOrig));
    
    if (sinThetaObs < 0.0001) {
        return velDir * cosThetaOrig;
    }
    
    vec3 perpDir = normalize(observedDir - cosThetaObs * velDir);
    return velDir * cosThetaOrig + perpDir * sinThetaOrig;
}

// ============================================================================
// Main
// ============================================================================

void main() {
    float beta = clamp(length(velocity), 0.0, 0.99);
    vec3 velDir = normalize(velocity + 0.0001);
    
    vec2 uv = fragTexCoord;
    
    // If no significant velocity, just passthrough
    if (beta < 0.01) {
        fragColor = texture(texture0, uv);
        return;
    }
    
    // Convert UV to viewing direction
    vec3 viewDir = normalize(vec3(
        (uv.x - 0.5) * 1.5,
        (uv.y - 0.5) * 1.5,
        -1.0
    ));
    
    // Apply inverse aberration to find original sampling direction
    vec3 origDir = inverseAberration(viewDir, velDir, beta);
    
    // Project original direction back to UV coordinates
    float projFactor = 1.5 / max(0.001, abs(origDir.z));
    vec2 sampleUV = vec2(
        origDir.x * projFactor + 0.5,
        origDir.y * projFactor + 0.5
    );
    sampleUV = clamp(sampleUV, 0.0, 1.0);
    
    // Sample the texture at the aberrated UV
    vec3 color = texture(texture0, sampleUV).rgb;
    
    // Calculate Doppler factor using the ORIGINAL view direction
    float cosTheta = dot(viewDir, velDir);
    float doppler = dopplerFactor(cosTheta, beta);
    
    // Apply Doppler color shift
    vec3 shiftedColor = applyDopplerShift(color, doppler);
    
    // Apply relativistic beaming
    float beamFactor = applyBeaming(cosTheta, beta);
    shiftedColor *= beamFactor;
    
    // Tone mapping (Reinhardt)
    shiftedColor = shiftedColor / (1.0 + shiftedColor);
    
    // Apply exposure
    shiftedColor *= exposure;
    
    fragColor = vec4(shiftedColor, 1.0);
}
