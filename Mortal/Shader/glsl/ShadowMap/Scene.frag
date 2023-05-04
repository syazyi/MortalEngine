#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLightDir;
layout(location = 4) in vec3 outViewDir;
layout (location = 5) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}


void main() {
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 ka = vec3(0.1, 0.1, 0.1 );
    float constant = 1.0;
    vec3 kd = vec3(0.5, 0.5, 0.5);
    float linear = 0.09;
    vec3 ks = vec3(1.0, 1.0, 1.0);
    float quadratic = 0.032;

    //attenuation
    float LightDistance = length(outLightDir);
    float attenuation = 1.0f / (constant + linear * LightDistance + quadratic * LightDistance * LightDistance);

    //ambient
    vec3 ambientLight = ka * vec3(lightColor);
    ambientLight *= attenuation;

    //diffuse
    vec3 N = normalize(outNormal);
    vec3 L = normalize(outLightDir);
    vec3 diffuseLight = max(dot(N, L), 0.0) * kd *  vec3(lightColor);
    diffuseLight *= attenuation;

    //spcularStrength
    vec3 V = normalize(outViewDir);
    vec3 H = normalize(L + V);
    vec3 specularLight = pow(max(dot(N, H), 0), 256) * ks * vec3(lightColor);
    specularLight *= attenuation;

    //shadow
    float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

    vec4 finalColor = vec4((ambientLight + diffuseLight + specularLight) * shadow, 1.0);
    //vec4 finalColor = vec4(ambientLight + diffuseLight + specularLight, 1.0);
    outColor = finalColor;
}
