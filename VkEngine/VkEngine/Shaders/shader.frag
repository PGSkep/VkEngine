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

void main()
{
	vec4 color = texture(fontSampler, inTexCoord).rgba;

	if(color.a == 0.0)
		discard;

	outColor = vec4(color + pushConstants.color);
}