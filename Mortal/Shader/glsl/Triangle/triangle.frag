#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D TexSampler;

void main() {
    outColor = texture(TexSampler, TexCoord_Sample);
}