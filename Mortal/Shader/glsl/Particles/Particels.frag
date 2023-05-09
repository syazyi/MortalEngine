#version 450
layout(binding = 1) uniform sampler2D ParticelsColor;
layout(location = 0)out vec4 outFragColor;
void main(){
    outFragColor = vec4(1.0, 1.0, 1.0, 1.0);
}