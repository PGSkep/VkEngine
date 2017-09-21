#include "Scene.h"

void Scene::Init()
{
	cameraPosition = { 0.0f, 0.0f, -2.f };
	cameraTarget = { 0.0f, 0.0f, 0.0f };
	cameraUp = { 0.0f, 1.0f, 0.0f };

	view = Math3D2::Mat4::GetLookAt(cameraPosition, cameraTarget, cameraUp);
}

void Scene::Update()
{
}

void Scene::ShutDown()
{
}
