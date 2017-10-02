#include "Scene.h"

void Scene::Init()
{
	cameraPosition = { 0.0f, 0.0f, -2.f };
	cameraTarget = { 0.0f, 0.0f, 0.0f };
	cameraUp = { 0.0f, 1.0f, 0.0f };

	view = Math3D::Mat4::GetLookAt(cameraPosition, cameraTarget, cameraUp);

	VkS::Text2D text;
	text.position = { -0.99f, -0.98f };
	text.scale = { 0.02f, 0.02f };
	text.spacing = { 0.04f, 0.1f };
	text.color = { 0.0f, 0.0f, 0.0f, 0.0f };
	text.text = "Hello World!\n0123...?";
	texts2D.push_back(text);
}

void Scene::Update(float _deltaTime)
{
	texts2D[0].text = "FPS = ";
	texts2D[0].text += std::to_string((uint64_t)(1.0f / _deltaTime));
}

void Scene::ShutDown()
{
}
