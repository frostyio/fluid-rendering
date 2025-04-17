#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 uv;
out vec3 fragPos;
out vec3 fragNorm;
out vec3 texCoord;

void main() {
    uv = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
    fragPos = vec3(0, 0, 0);
    fragNorm = vec3(0, 0, 0);
    texCoord = vec3(0, 0, 0);
}
