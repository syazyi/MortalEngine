#version 450

layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 outtexCoord;
void main(){
    outColor = texture(samplerCubeMap, outtexCoord);
}