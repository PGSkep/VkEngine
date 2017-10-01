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

float heightScale = -0.1;
float parallaxBias = 0.1;

vec2 parallaxMapping(vec2 _uv, vec3 _viewDirection)
{
	float height = 1.0 - texture(normalSampler, _uv).a;
	vec2 parallax = _viewDirection.xy * (height * (heightScale * 0.5) + parallaxBias) / _viewDirection.z;
	return _uv - parallax;
}

void main()
{
	vec3 V = normalize(inTangentViewPos - inTangentFragPos);
	V.x = -V.x;

	vec2 uv = parallaxMapping(abs(inTexCoord), V);

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