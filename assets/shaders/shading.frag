#version 330 core

layout(location = 0) out vec4 color;

in vec3 fragNorm;
in vec3 fragPos;
in vec3 texCoord;
in vec2 uv;

//

float fresnel(vec3 viewDir, vec3 normal) {
    float baseReflect = 0.02;
    float cosTheta = max(dot(viewDir, normal), 0.0);
    return baseReflect + (1.0 - baseReflect) * pow(1.0 - cosTheta, 5.0);
}

vec3 blinnPhongShading(vec3 norm, vec3 pos, vec3 light, vec3 view, vec3 specularColor, float alpha, vec3 ambientColor, vec3 diffuseColor) {
    norm = normalize(norm);

    vec3 lightDir = normalize(light - pos);
    vec3 viewDir = normalize(view - pos);

    vec3 diffuse = diffuseColor * max(dot(norm, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 specular = specularColor * pow(max(dot(viewDir, reflectDir), 0.0), alpha);

    return ambientColor + diffuse + specular;
}

uniform float nearPlane = 0.01;
uniform float farPlane = 1000;
float linearDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

//

/*
Shading Enum:
	0 - None
	1 - BlinnPhong
	2 - Solid ambient
*/
uniform int shading = 1;

/*
	0 = default/solid object (use varying fragNorm, fragPos)
	1 = screen-space water (use sampled normal & depth textures)
*/
uniform int materialType = 0;

// texs

// water
uniform sampler2D uBackgroundColorTex;
uniform sampler2D uOpaqueDepthTex;
uniform sampler2D uThicknessTex;
uniform samplerCube uSkyboxTex;
uniform sampler2D uDepthTex;

// multi-use texs
uniform bool hasNorm = false;
uniform sampler2D uNormalTex;

// mesh
uniform bool hasDiff = false;
uniform sampler2D uDiffTex;
uniform bool hasRough = false;
uniform sampler2D uRoughTex;

uniform mat4 uInvViewProj;
uniform mat4 uInvView;
uniform mat4 uInvProj;
uniform float uFovY;
uniform float uAspect;
uniform float uTime;

uniform float shininess = 10.0;
uniform vec3 lightPos = vec3(0., 0., 0.);
uniform vec3 viewPos = vec3(0., 0., 0.);

uniform vec3 ambientColor = vec3(0.4, 0.1, 0.1);
uniform vec3 diffuseColor = vec3(0.1, 0.1, 0.1);
uniform vec3 specularColor = vec3(1.0);

void main()
{
    vec3 norm = vec3(0.);
    vec3 pos = vec3(0.);
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 0);
    vec2 tiledUV = texCoord.xy * 10.0;

    if (materialType == 1) {
        // DEPTH
        float z = texture(uDepthTex, uv).r;
        if (z == 0.0) discard;
        vec2 ndc = uv * 2.0 - 1.0;

        // discard if behind something
        float opaqueDepth = texture(uOpaqueDepthTex, uv).r;
        float opaqueEyeDepth = -linearDepth(opaqueDepth);
        if (-z < opaqueEyeDepth) discard;

        // NORM
        vec3 packedNorm = texture(uNormalTex, uv).rgb;
        norm = packedNorm * 2.0 - 1.0;

        // POS
        float tanHalfFov = tan(uFovY * 0.5);
        pos.x = ndc.x * uAspect * tanHalfFov * -z;
        pos.y = ndc.y * tanHalfFov * -z;
        pos.z = -z;

        // VAR SETUP
        vec3 viewVec = normalize(viewPos - pos);
        float thickness = texture(uThicknessTex, uv).r;
        vec3 reflection = reflect(-viewVec, normalize(norm));

        // WATER THICKNESS
        float opacity = clamp(thickness / 50.0, 0.7, 0.8);

        // REFRACTION & REFLECTION
        vec2 refractOffset = norm.xy * 0.03;
        vec2 refractedUV = clamp(uv + refractOffset, 0.0, 1.0);
        vec3 refractedColor = texture(uBackgroundColorTex, refractedUV).rgb * 1.5;
        vec3 reflected = texture(uSkyboxTex, reflect(-viewVec, norm)).rgb * 0.6;
        float f = fresnel(viewVec, norm);

        // FINAL
        finalColor.rgb = mix(refractedColor, reflected, f);

        // finalColor.a = opacity;
        finalColor.a = 1;

        // fading
        float fade = smoothstep(0.01, 0.05, thickness);
        finalColor.rgb *= fade;
        finalColor.a *= fade;

        vec3 shallowColor = ambientColor;
        vec3 deepColor = diffuseColor;
        float depthFade = clamp(thickness / 50.0, 0.0, 1.0);
        vec3 absorptionColor = mix(shallowColor, deepColor, depthFade);
        finalColor.rgb *= clamp(absorptionColor, 0.0, 1.0);

        // foam
        vec2 texelSize = 1.0 / vec2(textureSize(uNormalTex, 0));
        vec3 center = norm;
        vec3 right = texture(uNormalTex, uv + vec2(texelSize.x, 0)).rgb * 2.0 - 1.0;
        vec3 up = texture(uNormalTex, uv + vec2(0, texelSize.y)).rgb * 2.0 - 1.0;
        float curvature = length(center - right) + length(center - up);

        float foam = smoothstep(0.08, 0.3, curvature);
        finalColor.rgb = mix(vec3(1.0), finalColor.rgb, 1.0 - foam);
    } else {
        norm = fragNorm;
        if (hasNorm) {
            norm = texture(uNormalTex, tiledUV).rgb * 2.0 - 1.0;
        }

        pos = fragPos;
    }

    if (shading == 1) {
        vec3 finalDiffuseColor = diffuseColor;
        if (hasDiff) {
            finalDiffuseColor = texture(uDiffTex, tiledUV).rgb;
        }

        float finalRoughness = 0.5;
        if (hasRough) {
            finalRoughness = texture(uRoughTex, tiledUV).r;
        }
        float finalShininess = mix(256.0, 8.0, finalRoughness);

        finalColor += vec4(blinnPhongShading(norm, pos, lightPos, viewPos, specularColor, finalShininess, ambientColor, finalDiffuseColor), 0.0);
    } else if (shading == 2) {
        finalColor += vec4(ambientColor, 0.0);
    }

    color = finalColor;
}
