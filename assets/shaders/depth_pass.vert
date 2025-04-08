#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int pointSize = 10;

out vec3 eyeSpacePos;

void main()
{
    vec3 fragPos = vec3(model * vec4(aPos, 1.0));

    vec4 viewPos = view * vec4(fragPos, 1.0);
    eyeSpacePos = viewPos.xyz;

    gl_Position = projection * viewPos;
    gl_Position.y = -gl_Position.y;

    gl_PointSize = pointSize;
}
