#ifndef	SCENE_H
#define SCENE_H

#include "Math3D.h"
#include "Input.h"

#include <vector>

struct Text2D
{
	Math3D::Vec2 position;
	Math3D::Vec2 centerOffset;
	float angle;
	Math3D::Vec2 scale;

	char* text;
};

struct Scene
{
public:
	Math3D::Vec3 cameraPosition;
	Math3D::Vec3 cameraTarget;
	Math3D::Vec3 cameraUp;

	Math3D::Mat4 view;

	std::vector<Text2D> texts2D;

	void Init();
	void Update();
	void ShutDown();
};

#endif