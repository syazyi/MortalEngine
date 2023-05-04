#version 450

layout(location = 0)in vec3 position;
layout(push_constant) uniform LightTransform{
	mat4 LightMVP;
}transform;
void main(){
	gl_Position = transform.LightMVP * vec4(position, 1.0);
}