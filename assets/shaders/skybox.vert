#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 invViewProj;

out vec3 TexCoords;
out float fragDepth;

void main() {
    TexCoords = (invViewProj * vec4(aPos, 1.0)).xyz;
    gl_Position = vec4(aPos, 1.0);
    gl_Position.y = -gl_Position.y;
    fragDepth = 1.0;
}
