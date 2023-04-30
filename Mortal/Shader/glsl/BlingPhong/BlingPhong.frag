#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLightDir;
layout(location = 4) in vec3 outViewDir;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D TexSampler;

layout(push_constant) uniform BlingPhongMaterial{
    vec3 lightColor;
    int q;
    vec3 ka;
    float constant;
    vec3 kd;
    float linear;
    vec3 ks;
    float quadratic;
} material;

void main() {
    //outColor = texture(TexSampler, TexCoord_Sample);
    //outColor = vec4(outNormal, 1.0);

    //attenuation
    float LightDistance = length(outLightDir);
    float attenuation = 1.0f / (material.constant + material.linear * LightDistance + material.quadratic * LightDistance * LightDistance);
    //ambient
    vec3 ambientLight = material.ka * vec3(material.lightColor);
    ambientLight *= attenuation;
    //diffuse
    vec3 N = normalize(outNormal);
    vec3 L = normalize(outLightDir);
    vec3 diffuseLight = max(dot(N, L), 0.0) * material.kd *  vec3(material.lightColor);
    diffuseLight *= attenuation;
    //spcularStrength
    vec3 V = normalize(outViewDir);
    vec3 H = normalize(L + V);
    vec3 specularLight = pow(max(dot(N, H), 0), material.q) * material.ks * vec3(material.lightColor);
    specularLight *= attenuation;

    vec4 finalColor = vec4(ambientLight + diffuseLight + specularLight, 1.0) * texture(TexSampler, TexCoord_Sample);
    //vec4 finalColor = vec4(ambientLight + diffuseLight + specularLight, 1.0);
    outColor = finalColor;
}
