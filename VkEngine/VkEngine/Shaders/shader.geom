#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout(location = 0) in vec2[] inSize;
layout(location = 1) in vec2[] inTexCoord;
layout(location = 2) in vec4[] inColor;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;

void main()
{
	vec2 center = gl_Position.xy;

	outTexCoord = inTexCoord[0];
	outColor = inColor[0];

	gl_Position = vec4(center.x - inSize[0].x, center.y - inSize[0].y, 0.0, 1.0);
	EmitVertex();

	gl_Position = vec4(center.x + inSize[0].x, center.y - inSize[0].y, 0.0, 1.0);
	EmitVertex();

	gl_Position = vec4(center.x - inSize[0].x, center.y + inSize[0].y, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();

	gl_Position = vec4(center.x + inSize[0].x, center.y - inSize[0].y, 0.0, 1.0);
	EmitVertex();

	gl_Position = vec4(center.x + inSize[0].x, center.y + inSize[0].y, 0.0, 1.0);
	EmitVertex();

	gl_Position = vec4(center.x - inSize[0].x, center.y + inSize[0].y, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();
}