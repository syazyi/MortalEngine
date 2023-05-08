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
	mat4 mv = ubo.view * ubo.model;
	vec4 outNormal4 =  ubo.normal * vec4(normal, 1.0);
	outNormal = outNormal4.xyz / outNormal4.w;
	vec4 posInView4 = mv * vec4(position, 1.0);
	vec3 posInView = posInView4.xyz / posInView4.w;

	vec4 lightPosInView4 =  ubo.view * vec4(ubo.lightPos, 1.0);
	vec3 lightPosInView = lightPosInView4.xyz / lightPosInView4.w;
	outLightDir = lightPosInView - posInView;
	outViewDir = -posInView;
	outShadowCoord = biasMat * lightMat.lightMVP * ubo.model * vec4(position, 1.0);
}