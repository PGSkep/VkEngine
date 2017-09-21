#ifndef	SCENE_H
#define SCENE_H

#include "Math3D.h"

struct Scene
{
public:
	Math3D2::Vec3 cameraPosition;
	Math3D2::Vec3 cameraTarget;
	Math3D2::Vec3 cameraUp;

	Math3D2::Mat4 view;

	void Init();
	void Update();
	void ShutDown();
};

#endif