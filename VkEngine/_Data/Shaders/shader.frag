#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseSampler;

void main()
{
	outColor = texture(diffuseSampler, inTexCoord) * vec4(1.0, 1.0, 1.0, 0.5);
}