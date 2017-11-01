#include "Scene.h"

#include <Windows.h>

#include "Globals.h"

//*
std::vector<Scene::GameObject> Scene::gameObjects;
Math3D::Mat4 Scene::viewProjectionData[2];

void ControlCamera(void* _camera)
{
	Scene::Camera* camera = (Scene::Camera*)_camera;

	if (GetAsyncKeyState('A'))
		camera->position.x += (float)Globals::deltaTime;
	if (GetAsyncKeyState('D'))
		camera->position.x -= (float)Globals::deltaTime;
	
	if (GetAsyncKeyState('Q'))
		camera->position.y += (float)Globals::deltaTime;
	if (GetAsyncKeyState('E'))
		camera->position.y -= (float)Globals::deltaTime;
	
	if (GetAsyncKeyState('W'))
		camera->position.z += (float)Globals::deltaTime;
	if (GetAsyncKeyState('S'))
		camera->position.z -= (float)Globals::deltaTime;

	camera->view = Math3D::Mat4::GetLookAt(camera->position, { 0.0f, 0.0f, 0.0f }, camera->up);
	Scene::viewProjectionData[0] = camera->view;
}
void ControlModel(void* _model)
{

}

void Destroy(void* _data)
{
	delete _data;
}

void Scene::Load()
{
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.position = { 0.0f, 0.0f, -2.f };

	camera.view = Math3D::Mat4::GetLookAt(camera.position, { 0.0f, 0.0f, 0.0f }, camera.up);
	camera.projection = Math3D::Mat4::GetPerspectiveProjection(45.0f, 800.0f, 600.0f, 0.1f, 100.0f);

	viewProjectionData[0] = camera.view;
	viewProjectionData[1] = camera.projection;

	modelMatrices.resize(128);

	Text2D text;
	text.position = { -0.97f, -0.95f };
	text.scale = { 0.025f, 0.04f };
	text.spacing = { 0.0375f, 0.07f };
	text.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	text.text = "Hello World!\n0123...?";
	texts2D.push_back(text);

	GameObject camera;

	gameObjects.push_back({});
	gameObjects[gameObjects.size() - 1].name = "Camera";
	gameObjects[gameObjects.size() - 1].datas = { GameObject::Data::GetData(new Camera, Destroy) };
	gameObjects[gameObjects.size() - 1].modules = { GameObject::Module::GetModule("Control Camera", gameObjects[gameObjects.size() - 1].datas[0].data, { ControlCamera }) };

	((Camera*)(gameObjects[gameObjects.size() - 1].datas[0].data))->position.z = -2.9f;
}
void Scene::Update()
{
	texts2D[0].text = "";
	texts2D[0].text += "UIOJKL controls the model, Ctrl rotates, shift scales\n"
		"QWEASD moves the camera\n";
	texts2D[0].text += "FPS = ";
	texts2D[0].text += std::to_string((uint64_t)(1.0f / Globals::deltaTime));

	if (GetAsyncKeyState(VK_ESCAPE))
		done = true;

	for (size_t iGameObj = 0; iGameObj != gameObjects.size(); ++iGameObj)
	{
		for (size_t iModule = 0; iModule != gameObjects[iGameObj].modules.size(); ++iModule)
		{
			for (size_t iFunction = 0; iFunction !=  gameObjects[iGameObj].modules[iModule].functions.size(); ++iFunction)
			{
				gameObjects[iGameObj].modules[iModule].functions[iFunction](gameObjects[iGameObj].modules[iModule].data);
			}
		}
	}

	// Update model
	{
		static Math3D::Mat4 transform = Math3D::Mat4::GetIdentity();

		if (GetAsyncKeyState(VK_SHIFT))
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetRotateZMatrix(-(float)Globals::deltaTime) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetRotateZMatrix(+(float)Globals::deltaTime) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetRotateYMatrix(+(float)Globals::deltaTime) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetRotateYMatrix(-(float)Globals::deltaTime) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetRotateXMatrix(+(float)Globals::deltaTime) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetRotateXMatrix(-(float)Globals::deltaTime) * transform;
		}
		else if (GetAsyncKeyState(VK_CONTROL))
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f + (float)Globals::deltaTime, 1.0f, 1.0f }) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f - (float)Globals::deltaTime, 1.0f, 1.0f }) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f + (float)Globals::deltaTime, 1.0f }) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f - (float)Globals::deltaTime, 1.0f }) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f, 1.0f + (float)Globals::deltaTime }) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetScaleMatrix({ 1.0f, 1.0f, 1.0f - (float)Globals::deltaTime }) * transform;
		}
		else
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D::Mat4::GetTranslateMatrix({ +(float)Globals::deltaTime, 0.0f, 0.0f }) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D::Mat4::GetTranslateMatrix({ -(float)Globals::deltaTime, 0.0f, 0.0f }) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, +(float)Globals::deltaTime, 0.0f }) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, -(float)Globals::deltaTime, 0.0f }) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, +(float)Globals::deltaTime }) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, -(float)Globals::deltaTime }) * transform;
		}

		modelMatrices[0] = transform;
	}
}

void Scene::ShutDown()
{
	for (size_t iGameObj = 0; iGameObj != gameObjects.size(); ++iGameObj)
	{
		for (size_t iData = 0; iData != gameObjects[iGameObj].datas.size(); ++iData)
		{
			gameObjects[iGameObj].datas[iData].destructor(
				gameObjects[iGameObj].datas[iData].data
			);
		}
	}
}
//*/
