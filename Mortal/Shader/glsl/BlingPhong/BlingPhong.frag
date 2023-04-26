#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLightDir;
layout(location = 4) in vec3 outViewDir;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D TexSampler;

layout(push_constant) uniform BlingPhongMaterial{
    vec4 lightColor;
    vec4 ka;
    vec4 kd;
    vec4 ks;
} material;

void main() {
    //outColor = texture(TexSampler, TexCoord_Sample);
    //outColor = vec4(outNormal, 1.0);

    //ambient
    vec3 ambientLight = vec3(material.ka * material.lightColor);

    //diffuse
    vec3 N = normalize(outNormal);
    vec3 L = normalize(outLightDir);
    vec3 diffuseLight = max(dot(N, L), 0.0) * vec3(material.kd * material.lightColor);

    //spcularStrength
    vec3 V = normalize(outViewDir);
    vec3 H = normalize(L + V);
    vec3 specularLight = pow(max(dot(N, H), 0), 256) * vec3(material.ks * material.lightColor);

    vec4 finalColor = vec4(ambientLight + diffuseLight + specularLight, 1.0) * texture(TexSampler, TexCoord_Sample);
    //vec4 finalColor = vec4(ambientLight + diffuseLight + specularLight, 1.0);
    outColor = finalColor;
}
