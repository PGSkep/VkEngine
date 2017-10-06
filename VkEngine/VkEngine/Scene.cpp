#include "Scene.h"

#include <Windows.h>

void Scene::Load()
{
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.position = { 0.0f, 0.0f, -2.f };

	camera.view = Math3D::Mat4::GetLookAt(camera.position, { 0.0f, 0.0f, 0.0f }, camera.up);
	camera.perspective = Math3D::Mat4::GetPerspectiveProjection(45.0f, 800.0f, 600.0f, 0.1f, 100.0f);

	viewProjectionData[0] = camera.view;
	viewProjectionData[1] = camera.perspective;

	modelMatrices.resize(128);

	Text2D text;
	text.position = { -0.90f, -0.85f };
	text.scale = { 0.05f, 0.06f };
	text.spacing = { 0.075f, 0.05f };
	text.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	text.text = "Hello World!\n0123...?";
	texts2D.push_back(text);
}
void Scene::Update(float _deltaTime)
{
	texts2D[0].text = "FPS = ";
	texts2D[0].text += std::to_string((uint64_t)(1.0f / _deltaTime));

	if (GetAsyncKeyState(VK_ESCAPE))
		done = true;

	// Update camera
	{
		if (GetAsyncKeyState('A'))
			camera.position.x += _deltaTime;
		if (GetAsyncKeyState('D'))
			camera.position.x -= _deltaTime;

		if (GetAsyncKeyState('Q'))
			camera.position.y += _deltaTime;
		if (GetAsyncKeyState('E'))
			camera.position.y -= _deltaTime;

		if (GetAsyncKeyState('W'))
			camera.position.z += _deltaTime;
		if (GetAsyncKeyState('S'))
			camera.position.z -= _deltaTime;
	}
	camera.view = Math3D::Mat4::GetLookAt(camera.position, { 0.0f, 0.0f, 0.0f }, camera.up);
	viewProjectionData[0] = camera.view;

	// Update model
	{
		static Math3D::Mat4 transform = Math3D::Mat4::GetIdentity();

		if (GetAsyncKeyState(VK_SHIFT))
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetRotateZMatrix(-_deltaTime) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetRotateZMatrix(+_deltaTime) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetRotateYMatrix(+_deltaTime) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetRotateYMatrix(-_deltaTime) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetRotateXMatrix(+_deltaTime) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetRotateXMatrix(-_deltaTime) * transform;
		}
		else if (GetAsyncKeyState(VK_CONTROL))
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f + _deltaTime, 1.0f, 1.0f }) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f - _deltaTime, 1.0f, 1.0f }) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f + _deltaTime, 1.0f }) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f - _deltaTime, 1.0f }) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f, 1.0f + _deltaTime }) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f, 1.0f - _deltaTime }) * transform;
		}
		else
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetTranslateMatrix({ +_deltaTime, 0.0f, 0.0f }) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetTranslateMatrix({ -_deltaTime, 0.0f, 0.0f }) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, +_deltaTime, 0.0f }) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, -_deltaTime, 0.0f }) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, +_deltaTime }) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, -_deltaTime }) * transform;
		}

		modelMatrices[0] = transform;
	}
}
