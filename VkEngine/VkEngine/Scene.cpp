#include "Scene.h"

void Scene::Init()
{
	cameraPosition = { 0.0f, 0.0f, -2.f };
	cameraTarget = { 0.0f, 0.0f, 0.0f };
	cameraUp = { 0.0f, 1.0f, 0.0f };

	view = Math3D::Mat4::GetLookAt(cameraPosition, cameraTarget, cameraUp);

	Text2D text;
	text.angle = 0.0f;
	text.centerOffset = { 0.0f, 0.0f };
	text.position = { 0.0f, 0.0f };
	text.scale = { 1.0f, 1.0f };
	text.text = "Hello World! 123...?";
	texts2D.push_back(text);
}

void Scene::Update()
{
}

void Scene::ShutDown()
{
}
