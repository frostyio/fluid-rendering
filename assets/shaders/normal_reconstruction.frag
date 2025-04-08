#version 330 core

in vec2 uv;
out vec3 fragNormal;

uniform sampler2D uFilteredDepth;
uniform float uFOV;
uniform float uScreenHeight;

void main() {
    vec2 texelSize = 1.0 / textureSize(uFilteredDepth, 0);

    float z = texture(uFilteredDepth, uv).r;
    float zx = texture(uFilteredDepth, uv + vec2(texelSize.x, 0)).r;
    float zy = texture(uFilteredDepth, uv + vec2(0, texelSize.y)).r;

    vec2 ndc = uv * 2.0 - 1.0;
    float aspect = float(textureSize(uFilteredDepth, 0).x) / uScreenHeight;
    float tanHalfFOV = tan(uFOV * 0.5);

    vec3 p = vec3(ndc.x * aspect * tanHalfFOV * -z,
            ndc.y * tanHalfFOV * -z,
            -z);
    vec3 px = vec3((ndc.x + texelSize.x * 2.0) * aspect * tanHalfFOV * -zx,
            ndc.y * tanHalfFOV * -zx,
            -zx);
    vec3 py = vec3(ndc.x * aspect * tanHalfFOV * -zy,
            (ndc.y + texelSize.y * 2.0) * tanHalfFOV * -zy,
            -zy);

    vec3 dx = normalize(px - p);
    vec3 dy = normalize(py - p);
    vec3 normal = normalize(cross(dy, dx));

    fragNormal = normal * 0.5 + 0.5;
}
