#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	layout(offset = 16) vec4 color;
} pushConstants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inTangentFragPos;
layout(location = 2) in vec3 inTangentLightPos;
layout(location = 3) in vec3 inTangentViewPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

float heightScale = -.1;
float numLayers = 48.0;

vec2 parallaxOcclusionMapping(vec2 _uv, vec3 _viewDirection) 
{
	float layerDepth = 1.0 / numLayers;
	float currLayerDepth = 0.0;
	vec2 deltaUV = _viewDirection.xy * heightScale / (_viewDirection.z * numLayers);
	vec2 currUV = _uv;
	float height = 1.0 - texture(normalSampler, currUV).a;
	for (int i = 0; i < numLayers; i++) {
		currLayerDepth += layerDepth;
		currUV -= deltaUV;
		height = 1.0 - texture(normalSampler, currUV).a;
		if (height < currLayerDepth) {
			break;
		}
	}
	vec2 prevUV = currUV + deltaUV;
	float nextDepth = height - currLayerDepth;
	float prevDepth = 1.0 - texture(normalSampler, prevUV).a - currLayerDepth + layerDepth;
	return mix(currUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

void main()
{
	vec3 V = normalize(inTangentViewPos - inTangentFragPos);
	V.x = -V.x;

	vec2 uv = abs(vec2(inTexCoord.x, inTexCoord.y));
	uv = parallaxOcclusionMapping(uv, V);

	if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
		discard;
	}

	vec3 N = normalize(texture(normalSampler, uv).rgb * 2.0 - 1.0);
	vec3 L = normalize(inTangentLightPos - inTangentFragPos);
	vec3 R = reflect(-L, N);
	vec3 H = normalize(L + V);

	vec3 color = texture(diffuseSampler, uv).rgb;
	vec3 ambient = 0.2 * color;
	vec3 diffuse = max(dot(L, N), 0.0) * color;
	vec3 specular = vec3(0.15) * pow(max(dot(N, H), 0.0), 32.0);

	outColor = vec4(ambient + diffuse + specular, 1.0f);
	return;
}