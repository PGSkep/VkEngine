#define _CRTDBG_MAP_ALLOC // memory leak detection
#include<crtdbg.h> // memory leak detection

#include "Globals.h"
#include "Gpu.h"
#include "Scene.h"
#include "GpuController.h"
#include "Timer.h"
#include "Input.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	Timer timer;
	timer.SetResolution(Timer::RESOLUTION::RESOLUTION_NANOSECONDS);
	timer.Play();

	double currentTime = timer.GetTime();
	double lastTime = currentTime;
	Globals::deltaTime = currentTime - lastTime;

	Input::Init();

	Gpu gpu;
	gpu.Init();

	DataPack dataPack;
	dataPack.textures = {
		DataPack::Texture::GetTexture("../_Data/Textures/font3.tga"),
		DataPack::Texture::GetTexture("../_Data/Textures/rocks diffuse.tga"),
		DataPack::Texture::GetTexture("../_Data/Textures/rocks normal.tga"),
	};

	dataPack.meshes = {
		DataPack::Mesh::GetMesh("../_Data/Models/cube.fbx", (Definitions::VERTEX_DATATYPE)(Definitions::VDT_X | Definitions::VDT_Y | Definitions::VDT_Z | Definitions::VDT_UV | Definitions::VDT_NORMAL | Definitions::VDT_TANGENT_BITANGENT)),
		DataPack::Mesh::GetMesh("../_Data/Models/Sphere.fbx", (Definitions::VERTEX_DATATYPE)(Definitions::VDT_X | Definitions::VDT_Y | Definitions::VDT_Z | Definitions::VDT_UV | Definitions::VDT_NORMAL | Definitions::VDT_TANGENT_BITANGENT)),
		DataPack::Mesh::GetMesh("../_Data/Models/monkey.fbx", (Definitions::VERTEX_DATATYPE)(Definitions::VDT_X | Definitions::VDT_Y | Definitions::VDT_Z | Definitions::VDT_UV | Definitions::VDT_NORMAL | Definitions::VDT_TANGENT_BITANGENT)),
	};

	Scene scene;
	scene.Load();

	GpuController gpuController;
	gpuController.Init(&gpu.device);
	gpuController.Load(scene, dataPack);
	gpuController.Setup(scene);

	while (scene.done == false)
	{
		currentTime = timer.GetTime();
		Globals::deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		scene.Update();
		gpuController.Compute();
		gpuController.Render(scene, (float)currentTime);
	}

	scene.ShutDown();

	gpuController.ShutDown();
	gpu.ShutDown();

	system("pause");

	return 0;
}