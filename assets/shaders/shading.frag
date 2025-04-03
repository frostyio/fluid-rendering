#version 330 core

layout(location = 0) out vec4 color;

in vec3 fragNorm;
in vec3 fragPos;
in vec3 texCoord;

//

vec3 blinnPhongShading(vec3 norm, vec3 pos, vec3 light, vec3 view, vec3 specularColor, float alpha, vec3 ambientColor, vec3 diffuseColor) {
    norm = normalize(norm);

    vec3 lightDir = normalize(light - pos);
    vec3 viewDir = normalize(view - pos);

    vec3 diffuse = diffuseColor * max(dot(norm, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 specular = specularColor * pow(max(dot(viewDir, reflectDir), 0.0), alpha);

    return ambientColor + diffuse + specular;
}

//

/*
Shading Enum:
	0 - None
	1 - BlinnPhong
*/
uniform int shading = 1;

uniform float shininess = 10.0;
uniform vec3 lightPos = vec3(0., 0., 0.);
uniform vec3 viewPos = vec3(0., 0., 0.);

uniform vec3 ambientColor = vec3(0.4, 0.1, 0.1);
uniform vec3 diffuseColor = vec3(0.1, 0.1, 0.1);
uniform vec3 specularColor = vec3(1.0);

void main()
{
    vec4 finalColor = vec4(0., 0., 0., 1.);

    if (shading == 1) {
        finalColor += vec4(blinnPhongShading(fragNorm, fragPos, lightPos, viewPos, specularColor, shininess, ambientColor, diffuseColor), 0.0);
    }

    color = finalColor;
}
