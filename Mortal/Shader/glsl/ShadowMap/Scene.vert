#version 450
layout(set = 0, binding = 0)uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normal;
	vec3 lightPos;
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
layout (location = 5) out vec4 outShadowCoord;

layout(push_constant)uniform LightMatrix{
	mat4 lightMVP;
} lightMat;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main(){
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
	fragColor = color;
	TexCoord_Sample = texCoord;
	mat3 mv = mat3(ubo.view * ubo.model);
	outNormal =  mat3(ubo.normal) * normal;
	vec3 posInView = mv * position;

	vec3 lightPosInView =  mat3(ubo.view) * ubo.lightPos;
	outLightDir = lightPosInView - posInView;
	outViewDir = -posInView;
	outShadowCoord = biasMat * lightMat.lightMVP * ubo.model * vec4(position, 1.0);
}