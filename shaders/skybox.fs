#version 330

in vec3 fragPosition;

uniform samplerCube skybox;
uniform vec3 velocity;

out vec4 fragColor;

void main() {
    vec3 dir = normalize(fragPosition);
    vec3 velDir = normalize(velocity + 0.0001);
    float beta = min(length(velocity), 0.9);

    float ct = dot(dir, velDir);
    float ctStar = clamp((ct - beta) / (1.0 - beta * ct), -1.0, 1.0);

    vec3 perp = dir - velDir * ct;
    float plen = length(perp);
    vec3 sampleDir = (plen > 1e-4)
        ? velDir * ctStar + (perp / plen) * sqrt(max(0.0, 1.0 - ctStar * ctStar))
        : dir;

    vec3 color = texture(skybox, sampleDir).rgb;

    float gamma = 1.0 / sqrt(max(0.001, 1.0 - beta * beta));
    float delta = 1.0 / (gamma * max(1e-3, 1.0 - beta * ctStar));
    float shift = clamp((delta - 1.0) / (delta + 1.0), -1.0, 1.0);

    float boost = delta * delta;
    float lift = 1.0 / (1.0 + 0.5 * log2(max(boost, 1.0)));

    color = pow(clamp(color * boost, 0.0, 1.0), vec3(lift));
    color *= vec3(1.0 - 0.08 * shift, 1.0 + 0.02 * shift, 1.0 + 0.10 * shift);
    color += vec3(0.045, 0.047, 0.051) * max(boost - 1.0, 0.0);

    fragColor = vec4(color, 1.0);
}
