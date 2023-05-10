#version 450
layout(binding = 1) uniform sampler2D ParticelsColor;
layout(location = 0)out vec4 outFragColor;
void main(){
    outFragColor.rgb = texture(ParticelsColor, gl_PointCoord).rgb;
}