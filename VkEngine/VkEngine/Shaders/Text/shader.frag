#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	layout(offset = 32)
	vec4 color;
} pushConstants;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D fontSampler;

float blending = 0.05;
float hardness = 5.0;

void main()
{
	vec4 color = texture(fontSampler, inTexCoord).rgba;

	if(color.a == 0.0)
		discard;

	vec3 rgb = vec3(0.0f, 0.0f, 0.0f) * (color.a + blending) + pushConstants.color.rgb * (color.a - blending);
	float alpha = color.a * hardness;

	outColor = vec4(rgb, alpha);
}