// FOR DEPTH

#version 330 core
in vec2 uv;
out vec4 fragColor;

uniform sampler2D uTexture;

void main() {
    float d = texture(uTexture, uv).r;
    float visual = clamp(d / 100.0, 0.0, 1.0);
    fragColor = vec4(visual, visual, visual, 1.0);
    // fragColor = vec4(1, 0, 0, .1);
    // if (visual > 0)
    //     fragColor = vec4(1, 0, 0, 1);
    // else if (visual == 0)
    //     fragColor = vec4(0, 0, 0, 1);
    // else if (visual < 0)
    //     fragColor = vec4(0, 0, 1, 1);
    //
    fragColor = texture(uTexture, uv);
}

// FOR NORMALS

// #version 330 core
// in vec2 uv;
// out vec4 fragColor;

// uniform sampler2D uTexture;

// void main() {
//     vec3 normal = texture(uTexture, uv).rgb;
//     normal = normal * 2.0 - 1.0;
//     fragColor = vec4(normal * 0.5 + 0.5, 0.5);
// }
