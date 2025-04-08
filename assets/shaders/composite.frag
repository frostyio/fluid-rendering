#version 330 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D uScene;
uniform sampler2D uTrans;
uniform sampler2D uPost;

void main() {
    vec4 scene = texture(uScene, uv);
    vec4 trans = texture(uTrans, uv);
    vec4 post = texture(uPost, uv);

    vec4 over1 = mix(trans, post, post.a);
    vec4 over2 = mix(scene, over1, over1.a);

    fragColor = vec4(over2.rgb, 1.0);
}
