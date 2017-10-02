#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout(push_constant) uniform PushConstants
{
	layout(offset = 16)
	vec4 texCoord;
} pushConstants;

layout(location = 0) in vec2[] inPosition;
layout(location = 1) in vec2[] inSize;

layout(location = 0) out vec2 outTexCoord;

void main()
{

	gl_Position = vec4(
		inPosition[0].x - inSize[0].x, 
		inPosition[0].y - inSize[0].y, 
		0.0, 1.0);
	outTexCoord = pushConstants.texCoord.xy;
	EmitVertex();

	gl_Position = vec4(
		inPosition[0].x + inSize[0].x,
		inPosition[0].y - inSize[0].y,
		0.0, 1.0);
	outTexCoord = pushConstants.texCoord.zy;
	EmitVertex();

	gl_Position = vec4(
		inPosition[0].x - inSize[0].x,
		inPosition[0].y + inSize[0].y,
		0.0, 1.0);
	outTexCoord = pushConstants.texCoord.xw;
	EmitVertex();

	gl_Position = vec4(
		inPosition[0].x + inSize[0].x,
		inPosition[0].y + inSize[0].y,
		0.0, 1.0);
	outTexCoord = pushConstants.texCoord.zw;
	EmitVertex();

	EndPrimitive();
}