#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	vec2 position;
	vec2 size;
	vec2 texCoord;
	vec4 color;
} pushConstants;

layout(location = 0) out vec2 outSize;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec4 outColor;

void main()
{
	gl_Position = vec4(pushConstants.position, 1.0, 1.0);
	outSize = pushConstants.size;
	outTexCoord = pushConstants.texCoord;
	outColor = pushConstants.color;
}