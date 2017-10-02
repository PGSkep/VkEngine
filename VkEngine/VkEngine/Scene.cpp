#include "Scene.h"

void Scene::Init()
{
	cameraPosition = { 0.0f, 0.0f, -2.f };
	cameraTarget = { 0.0f, 0.0f, 0.0f };
	cameraUp = { 0.0f, 1.0f, 0.0f };

	view = Math3D::Mat4::GetLookAt(cameraPosition, cameraTarget, cameraUp);

	VkS::Text2D text;
	text.position = { -0.90f, -0.85f };
	text.scale = { 0.05f, 0.06f };
	text.spacing = { 0.075f, 0.05f };
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
