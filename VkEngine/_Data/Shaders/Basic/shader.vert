#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	uint modelMatrixIndex;
	float red;
	float green;
	float blue;
} pushConstants;

layout(set = 0, binding = 0) uniform ViewProjection
{
	mat4 view;
	mat4 projection;
} vp;
layout(set = 0, binding = 1) uniform ModelMatrices
{
	mat4 matrices[1];
} modelMatrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;

void main()
{
	gl_Position = vp.projection * vp.view * modelMatrices.matrices[pushConstants.modelMatrixIndex] * vec4(inPosition, 1.0);
	outTexCoord = inTexCoord;
	outNormal = inNormal;
}