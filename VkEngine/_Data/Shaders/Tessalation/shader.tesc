#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices = 3) out;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inTexCoord[];
layout(location = 2) in vec3 inNormal[];

layout(location = 0) out vec3 outPosition[];
layout(location = 1) out vec2 outTexCoord[];
layout(location = 2) out vec3 outNormal[];

float GetTessLevel(float Distance0, float Distance1)
{
	float result = 3.0;
	float AvgDistance = (Distance0 + Distance1) / 2.0;

	if (AvgDistance <= 2.0)
		result = 10.0;

	if (AvgDistance <= 5.0)
		result = 7.0;

	return result;
}

void main()
{
	vec3 eyeWorldPos = vec3(0.0, 0.0, 0.0);

	// Set the control points of the output patch
	outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
	outTexCoord[gl_InvocationID] = inTexCoord[gl_InvocationID];
	outNormal[gl_InvocationID] = inNormal[gl_InvocationID];

	// Calculate the distance from the camera to the three control points
    float EyeToVertexDistance0 = distance(eyeWorldPos, outPosition[0]);
    float EyeToVertexDistance1 = distance(eyeWorldPos, outPosition[1]);
    float EyeToVertexDistance2 = distance(eyeWorldPos, outPosition[2]);

    // Calculate the tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}