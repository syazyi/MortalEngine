#version 450
layout(set = 0, binding = 0)uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 TexCoord_Sample;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outLightDir;
layout(location = 4) out vec3 outViewDir;


void main(){
	gl_Position = ubo.proj * ubo.view * vec4(position, 1.0);
	fragColor = color;
	TexCoord_Sample = texCoord;

	mat3 vp = mat3(ubo.proj * ubo.view);
	outNormal =  vp * normal;
	vec3 posInView = vp * position;

	vec3 lightPosInView =  mat3(ubo.proj * ubo.view * ubo.model) * vec3(5.0, 5.0, 5.0);
	outLightDir = lightPosInView - posInView;
	outViewDir = posInView;
}