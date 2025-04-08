#version 330 core
out float fragColor;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0)
        discard;

    fragColor = 1.0;
}
