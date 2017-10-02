#ifndef	SCENE_H
#define SCENE_H

#include "Math3D.h"
#include "Input.h"
#include "Vk.h"

#include <vector>

struct Scene
{
public:
	Math3D::Vec3 cameraPosition;
	Math3D::Vec3 cameraTarget;
	Math3D::Vec3 cameraUp;

	Math3D::Mat4 view;

	std::vector<VkS::Text2D> texts2D;

	void Init();
	void Update(float _deltaTime);
	void ShutDown();
};

#endif