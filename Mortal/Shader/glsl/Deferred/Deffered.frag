#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLightDir;
layout(location = 4) in vec3 outViewDir;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoSampler;

/*
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
*/

void main() {
    outColor = texture(normalSampler, TexCoord_Sample);
}
