#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	layout(offset = 16) vec4 color;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D diffuse;

void main()
{
	vec4 textureColor = texture(diffuse, inTexCoord).rgba;
	//outColor = vec4(inTexCoord.x, inTexCoord.y, 1.0, 1.0);
	outColor = textureColor;
}