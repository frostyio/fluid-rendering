#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in float fragDepth;
uniform samplerCube skybox;

void main() {
    gl_FragDepth = fragDepth;
    FragColor = texture(skybox, normalize(TexCoords));
}
