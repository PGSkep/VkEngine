// Source: https://github.com/SaschaWillems/Vulkan
#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(triangles, equal_spacing, ccw) in;

layout(location = 0) in vec3	inPosition[];
layout(location = 1) in vec2	inTexCoord[];
layout(location = 2) in vec3	inNormal[];

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
   	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
   	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
	vec3 position = vec3(0.0, 0.0, 0.0);
	float gDispFactor = 3.0;
	mat4 gVP = mat4(1.34444, 0.0, 0.0, 0.0,		0.0, 1.79259, 0.0, 0.0,		0.0, 0.0, -1.0001, -1.0,		0.0, 0.0, -0.10001, 0.0);

	// Interpolate the attributes of the output vertex using the barycentric coordinates

	outTexCoord = interpolate2D(inTexCoord[0], inTexCoord[1], inTexCoord[2]);
	outNormal = interpolate3D(inNormal[0], inNormal[1], inNormal[2]);
	outNormal = normalize(outNormal);
	position = interpolate3D(inPosition[0], inPosition[1], inPosition[2]);

	// Displace the vertex along the normal

	float Displacement = 0.0;	//texture(gDisplacementMap, TexCoord_FS_in.xy).x;
	position += outNormal * Displacement * gDispFactor;
	gl_Position = gVP * vec4(position, 1.0);
}