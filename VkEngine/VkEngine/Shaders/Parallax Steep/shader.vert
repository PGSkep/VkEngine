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
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outTangentFragPos;
layout(location = 2) out vec3 outTangentLightPos;
layout(location = 3) out vec3 outTangentViewPos;

void main()
{
	mat4 modelMatrix = modelMatrices.matrices[pushConstants.modelMatrixIndex];

	vec3 cameraPosition = vec3(
		vp.view[0][2] * vp.view[3][2],
		vp.view[1][2] * vp.view[3][2],
		vp.view[2][2] * vp.view[3][2]);

	vec3 T = normalize(mat3(modelMatrix) * inTangent);
	vec3 B = normalize(mat3(modelMatrix) * inBiTangent);
	vec3 N = normalize(mat3(modelMatrix) * inNormal);
	mat3 TBN = transpose(mat3(T, B, N));

	gl_Position = vp.projection * vp.view * modelMatrix * vec4(inPosition, 1.0);
	outTexCoord = inTexCoord;

	outTangentFragPos = vec3(modelMatrix * vec4(inPosition, 1.0));
	outTangentFragPos  = TBN * outTangentFragPos;
	outTangentLightPos = TBN * vec3(pushConstants.red, pushConstants.green, pushConstants.blue);
	outTangentViewPos = TBN * cameraPosition;
}