#version 450

layout(set = 0, binding = 0)uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec2 normal;


layout(location = 0) out vec3 outtexCoord;
void main(){
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
	outtexCoord = position;
}