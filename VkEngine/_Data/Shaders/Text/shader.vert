#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	vec2 position;
	vec2 size;
} pushConstants;

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec2 outSize;

void main()
{
	gl_Position = vec4(pushConstants.position.x, pushConstants.position.y, 0.0, 1.0);
	outPosition = pushConstants.position;
	outSize = pushConstants.size;
}