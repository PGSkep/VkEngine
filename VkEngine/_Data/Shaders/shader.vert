#version 450
#extension GL_ARB_separate_shader_objects : enable

// Push Constant
layout(push_constant) uniform PushConstants
{
	float aspect;
} pushConstants;

// Vertex
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;

// Instance
layout (location = 2) in vec2 instancePosition;
layout (location = 3) in vec2 instanceScale;

// Output
layout(location = 0) out vec2 outTexCoord;

void main()
{
	vec2 vPosition = vec2(inPosition.x * instanceScale.x, inPosition.y * instanceScale.y);

	gl_Position = vec4(vPosition + instancePosition, 0.0, 1.0);
	gl_Position.y *= pushConstants.aspect;

	outTexCoord = inTexCoord;
}