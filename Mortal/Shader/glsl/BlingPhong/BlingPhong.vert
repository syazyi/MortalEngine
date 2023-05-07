#version 450
layout(set = 0, binding = 0)uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normalMat;
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


void main(){
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
	fragColor = color;
	TexCoord_Sample = texCoord;

	mat4 mv = ubo.view * ubo.model;
	vec4 outNormal4 =  ubo.normalMat * vec4(normal, 1.0);
	outNormal =  outNormal4.xyz / outNormal4.w;
	vec4 posInView4 = mv * vec4(position, 1.0);
	posInView4 = posInView4.xyzw / posInView4.w;

	vec4 lightPosInView = ubo.view * vec4(ubo.lightPos, 1.0);
	lightPosInView = lightPosInView.xyzw / lightPosInView.w;
	vec3 lightDir = lightPosInView.xyz - posInView4.xyz;
	outLightDir = lightDir;
	outViewDir = -posInView4.xyz;
}