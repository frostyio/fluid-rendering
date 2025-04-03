#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tex;

out vec3 fragPos;
out vec3 fragNorm;
out vec3 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // to world space
    fragPos = vec3(model * vec4(pos, 1.0));

    // normal in world space
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    fragNorm = normalMatrix * norm;

    // to screen space
    gl_Position = projection * view * vec4(fragPos, 1.0);
    gl_Position.y = -gl_Position.y;

    // passing along
    texCoord = tex;
}
