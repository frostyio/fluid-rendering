#version 330 core

in vec3 eyeSpacePos;
layout(location = 0) out float fragDepth;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0)
        discard;

    if (eyeSpacePos.z > -0.001) discard;

    fragDepth = -eyeSpacePos.z;
}
