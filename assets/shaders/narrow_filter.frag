#version 330 core

in vec2 uv;
out float fragColor;

uniform sampler2D uDepthTex;
uniform float uDelta;
uniform float uMu;
uniform float uWorldSigma;
uniform float uFOV;
uniform float uScreenHeight;

const int KERNEL_RADIUS = 8;

float gaussianWeight(vec2 a, vec2 b, float sigma) {
    return exp(-dot(b - a, b - a) / (2.0 * sigma * sigma));
}

void main() {
    float zi = texture(uDepthTex, uv).r;
    if (zi == 0.0) discard;

    float sigma_i = (uScreenHeight * uWorldSigma) / (2.0 * abs(zi) * tan(uFOV * 0.5));
    float kernelRadius = 3.0 * sigma_i;
    vec2 texelSize = 1.0 / textureSize(uDepthTex, 0);

    float sumWeights = 0.0;
    float sumDepths = 0.0;

    float deltaLow = uDelta;
    float deltaHigh = uDelta;

    float kernelRadiusPx = kernelRadius * texelSize.y;

    for (int dy = -KERNEL_RADIUS; dy <= KERNEL_RADIUS; ++dy) {
        for (int dx = -KERNEL_RADIUS; dx <= KERNEL_RADIUS; ++dx) {
            vec2 offset = vec2(float(dx), float(dy)) * texelSize;
            vec2 neighborUV = uv + offset;

            if (dot(offset, offset) > kernelRadiusPx * kernelRadiusPx)
                continue;

            if (neighborUV.x < 0.0 || neighborUV.x > 1.0 ||
                    neighborUV.y < 0.0 || neighborUV.y > 1.0)
                continue;

            float zj = texture(uDepthTex, neighborUV).r;

            vec2 mirrorUV = uv + (uv - neighborUV);
            if (mirrorUV.x < 0.0 || mirrorUV.x > 1.0 ||
                    mirrorUV.y < 0.0 || mirrorUV.y > 1.0)
                continue;

            float zk = texture(uDepthTex, mirrorUV).r;
            if (zj > zi + uDelta || zk > zi + uDelta)
                continue;

            if (zj == 0.0 || zk == 0.0)
                continue;

            float weight = gaussianWeight(uv, neighborUV, sigma_i);

            if (zj >= zi - deltaLow && zj <= zi + deltaHigh) {
                deltaLow = max(deltaLow, zi - zj + uDelta);
                deltaHigh = max(deltaHigh, zj - zi + uDelta);
            }

            float f;
            if (zj >= zi - deltaLow && zj <= zi + deltaHigh)
                f = zj;
            else
                f = zi - uMu;

            sumWeights += weight;
            sumDepths += weight * f;
        }
    }

    fragColor = sumDepths / max(sumWeights, 0.0001);
}
