#version 330

// Alcubierre Warp Drive - 引力透镜与泡壁光学效应
// 泡壁固定在屏幕中心（飞船内部视角），速度只影响多普勒和聚束方向

in vec2 fragTexCoord;

uniform sampler2D texture0;       // 输入纹理（bloom后的场景）
uniform vec3 velocity;            // 速度方向与大小
uniform float warpFactor;         // 曲速因子 (1.0=静止, >1=超光速)
uniform float bubbleRadius;       // 曲速泡半径 (屏幕空间, 0.3~0.8)
uniform float wallThickness;      // 泡壁厚度
uniform float exposure;           // 曝光

out vec4 fragColor;

// ============================================================================
// 核心光学效应
// ============================================================================

// 泡壁折射 - 引力透镜核心扭曲
// 泡壁固定在屏幕中心，不随速度方向变化
vec2 applyBubbleRefraction(vec2 uv, float warpF) {
    vec2 center = vec2(0.5);
    vec2 delta = uv - center;
    float dist = length(delta);

    if (dist < 0.001) return uv;

    vec2 radialDir = delta / dist;

    // 泡壁影响区域: 以 bubbleRadius 为中心的环形带
    float wallInner = max(0.01, bubbleRadius - wallThickness);
    float wallOuter = min(0.99, bubbleRadius + wallThickness);

    // 在环形带内最强
    float wallFactor = smoothstep(wallInner - 0.08, wallInner, dist) *
                       (1.0 - smoothstep(wallOuter, wallOuter + 0.08, dist));

    // 爱因斯坦角偏移
    float einsteinAngle = warpF * 0.1;
    float distortion = einsteinAngle * wallFactor / (dist + 0.02);

    // 径向扭曲 - 光线向泡壁偏折
    uv -= radialDir * distortion * 0.5;

    // 切向漩涡 - 产生弧形畸变
    vec2 tangentDir = vec2(-delta.y, delta.x) / dist;
    float tangentialDistortion = einsteinAngle * wallFactor * 0.12;
    uv += tangentDir * tangentialDistortion;

    return uv;
}

// 色差效应 - 不同波长折射率不同
vec3 applyChromaticAberration(vec2 uv, float warpF) {
    float distortion = warpF * 0.006;

    float r = texture(texture0, uv + vec2(distortion, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(distortion, 0.0)).b;

    return vec3(r, g, b);
}

// 多普勒频移 - 基于速度方向
vec3 applyDopplerShift(vec3 color, vec2 uv, vec3 velDir, float warpF) {
    // 屏幕中心为"前方"，越靠近边缘越"侧方/后方"
    vec2 fromCenter = uv - 0.5;
    float distFromCenter = length(fromCenter);

    // 速度在屏幕上的投影方向
    vec3 normVel = normalize(velDir + 0.001);
    vec2 velProj = normalize(normVel.xy + 0.001);

    // 视线与速度方向的余弦
    float cosTheta;
    if (distFromCenter > 0.001) {
        vec2 viewProj = normalize(fromCenter);
        cosTheta = dot(viewProj, velProj);
    } else {
        cosTheta = 1.0; // 正前方
    }

    // 有效 beta
    float beta = clamp((warpF - 1.0) / warpF, 0.0, 0.99);
    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));

    // 多普勒因子
    float doppler = 1.0 / (gamma * (1.0 - beta * cosTheta + 0.001));

    // 色温映射
    vec3 shifted;
    if (doppler > 1.0) {
        // 蓝移: 增强蓝
        shifted = vec3(
            color.r * 0.8,
            color.g * doppler * 0.9,
            color.b * doppler * 1.1
        );
    } else {
        // 红移: 增强红
        float redShift = 1.0 / max(0.1, doppler);
        shifted = vec3(
            color.r * redShift * 1.0,
            color.g * doppler * 0.85,
            color.b * doppler * 0.6
        );
    }

    return clamp(shifted, 0.0, 5.0);
}

// 聚束效应 - 向速度方向汇聚
float applyBeaming(vec2 uv, vec3 velDir, float warpF) {
    vec2 fromCenter = uv - 0.5;
    float dist = length(fromCenter);
    if (dist < 0.001) return pow(warpF, 2.0);

    vec3 normVel = normalize(velDir + 0.001);
    vec2 velProj = normalize(normVel.xy + 0.001);
    vec2 viewProj = normalize(fromCenter);

    float cosTheta = dot(viewProj, velProj);
    float beta = clamp((warpF - 1.0) / warpF, 0.0, 0.99);
    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));
    float doppler = 1.0 / (gamma * (1.0 - beta * cosTheta + 0.001));

    return pow(max(doppler, 0.0), 2.5);
}

// 前方聚光亮斑 - 固定在屏幕中心
vec3 forwardGlow(vec2 uv, float warpF) {
    float dist = length(uv - 0.5);
    float glowRadius = 0.25 / max(warpF, 2.0);
    float glow = exp(-dist * dist / (glowRadius * glowRadius));

    return vec3(0.4, 0.6, 1.0) * glow * (warpF - 1.0) * 0.6;
}

// 弓形波辐射 - 从中心向外扩散
vec3 bowWave(vec2 uv, float warpF) {
    vec2 fromCenter = uv - 0.5;
    float dist = length(fromCenter);
    if (dist < 0.001) return vec3(0.0);

    vec2 dir = fromCenter / dist;

    // 环形辐射，前方更强
    float forwardCos = dir.y; // 假设上方为前方
    float forwardFactor = max(forwardCos, 0.0);

    float radialFade = exp(-dist * 3.0);
    float coneShape = pow(forwardFactor, 1.5);

    return vec3(0.3, 0.5, 0.9) * coneShape * radialFade * (warpF - 1.0) * 0.25;
}

// 泡壁环形亮带 - 固定在屏幕中心
float bubbleWallGlow(vec2 uv, float warpF) {
    float dist = length(uv - 0.5) * 2.0;
    float wallInner = max(0.01, bubbleRadius - wallThickness);
    float wallOuter = min(0.99, bubbleRadius + wallThickness);

    // 环形高斯分布
    float wallCenter = (wallInner + wallOuter) * 0.5;
    float wallSigma = (wallOuter - wallInner) * 0.5 + 0.02;
    float glow = exp(-pow(dist - wallCenter, 2.0) / (2.0 * wallSigma * wallSigma));

    return glow * warpF * 0.35;
}

// 泡壁内部区域 - 平坦时空区
float bubbleInterior(vec2 uv) {
    float dist = length(uv - 0.5) * 2.0;
    float interiorEdge = max(0.01, bubbleRadius - wallThickness);
    return 1.0 - smoothstep(interiorEdge - 0.05, interiorEdge, dist);
}

// ============================================================================
// 主函数
// ============================================================================

void main() {
    float speed = length(velocity);
    float effectiveWarp = max(warpFactor, speed);

    // 静止或低速时直接输出
    if (effectiveWarp < 1.05) {
        fragColor = texture(texture0, fragTexCoord);
        return;
    }

    vec3 velDir = normalize(velocity + 0.0001);
    vec2 uv = fragTexCoord;

    // --- 第一步：泡壁折射扭曲（固定中心） ---
    vec2 distortedUV = applyBubbleRefraction(uv, effectiveWarp);
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    // --- 第二步：色差采样 ---
    vec3 color = applyChromaticAberration(distortedUV, effectiveWarp);

    // --- 第三步：多普勒频移（依赖速度方向） ---
    color = applyDopplerShift(color, distortedUV, velDir, effectiveWarp);

    // --- 第四步：聚束效应（依赖速度方向） ---
    float beam = applyBeaming(distortedUV, velDir, effectiveWarp);
    color *= beam;

    // --- 第五步：前方聚光亮斑（固定中心） ---
    color += forwardGlow(uv, effectiveWarp);

    // --- 第六步：弓形波辐射 ---
    color += bowWave(uv, effectiveWarp);

    // --- 第七步：泡壁环形发光（固定中心） ---
    float wallGlow = bubbleWallGlow(uv, effectiveWarp);
    color += vec3(0.3, 0.5, 1.0) * wallGlow;

    // --- 第八步：泡壁内部略微提亮（平坦时空区） ---
    float interior = bubbleInterior(uv);
    color = mix(color, color * 1.1, interior * 0.3);

    // --- 第九步：色调映射 + 曝光 ---
    color = color / (1.0 + color);  // Reinhard
    color *= exposure;

    fragColor = vec4(color, 1.0);
}
