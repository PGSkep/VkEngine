#define _CRTDBG_MAP_ALLOC // memory leak detection
#include<crtdbg.h> // memory leak detection

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
	double deltaTime = currentTime - lastTime;

	Input::Init();

	Gpu gpu;
	gpu.Init();

	Scene scene;
	scene.Load();

	GpuController gpuController;
	gpuController.Init(&gpu.device);
	gpuController.Load(scene);
	gpuController.Setup(scene);

	while (scene.done == false)
	{
		currentTime = timer.GetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		scene.Update((float)deltaTime);
		gpuController.Compute();
		gpuController.Render(scene, (float)currentTime, (float)deltaTime);
	}

	scene.ShutDown();

	gpuController.ShutDown();
	gpu.ShutDown();

	system("pause");

	return 0;
}