#ifndef	SCENE_H
#define SCENE_H

#include "Math3D.h"

struct Scene
{
public:
	Math3D::Vec3 cameraPosition;
	Math3D::Vec3 cameraTarget;
	Math3D::Vec3 cameraUp;

	Math3D::Mat4 view;

	void Init();
	void Update();
	void ShutDown();
};

#endif