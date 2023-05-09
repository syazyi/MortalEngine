#version 450
layout(location = 0) in vec3 inPos;
layout(set = 0, binding = 0) uniform UBO{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;


void main(){
	const float spriteSize = 0.005; // Point size influenced by mass (stored in inPos.w);

	vec4 eyePos = ubo.view * vec4(inPos.x, inPos.y, inPos.z, 1.0); 
	vec4 projectedCorner = ubo.proj * vec4(0.5 * spriteSize, 0.5 * spriteSize, eyePos.z, eyePos.w);
	gl_PointSize = clamp(projectedCorner.x / projectedCorner.w, 1.0, 128.0);

    gl_Position = ubo.proj * ubo.view * vec4(inPos, 1.0);
}