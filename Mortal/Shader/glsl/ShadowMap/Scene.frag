#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 TexCoord_Sample;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLightDir;
layout(location = 4) in vec3 outViewDir;
layout (location = 5) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

float TextureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float FilterPCF(vec4 shadowCoord){
    ivec2 shadowMapSize = textureSize(shadowMap, 0);
    float scale = 2.0f;
    float dx = scale * 1.0f / shadowMapSize.x;
    float dy = scale * 1.0f / shadowMapSize.y;

    int count = 0;
    int filterRange = 2;
    float shadow = 0.0;
    //(filterRange * 2 + 1) * (filterRange * 2 + 1), find apart scale pixel
    for(int i = -filterRange; i <= filterRange; i++){
        for(int j = -filterRange; j <= filterRange; j++){
            shadow += TextureProj(shadowCoord, vec2(i*dx, j*dy));
            count++;
        }        
    }
    return shadow / count;
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
    //float shadow = TextureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
    float shadow = FilterPCF(inShadowCoord / inShadowCoord.w);

    vec4 finalColor = vec4((ambientLight + diffuseLight + specularLight) * shadow, 1.0);
    //vec4 finalColor = vec4((specularLight + ambientLight), 1.0);
    //vec4 finalColor = texture( shadowMap, (inShadowCoord / inShadowCoord.w).st);
    outColor = finalColor;
}
