#version 450

vec3 positions[3] = vec3[](
	vec3(0.0, -0.5, 0.0),
	vec3(0.5, 0.5, 0.0),
	vec3(-0.5, 0.5, 0.0)
);

vec3 color[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main(){
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	fragColor = color[gl_VertexIndex];
}