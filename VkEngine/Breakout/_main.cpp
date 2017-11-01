#define _CRTDBG_MAP_ALLOC // memory leak detection
#include<crtdbg.h> // memory leak detection


#include "../_Common/Vulkan/Gpu.h"
#include "../_Common/Timer.h"
#include "../_Common/Input.h"

#include "GameplayBreakout.h"
#include "GpuBreakout.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	Timer timer;
	timer.SetResolution(Timer::RESOLUTION::RESOLUTION_NANOSECONDS);
	timer.Play();
	double currentTime;
	double lastTime;
	double deltaTime;

	GameplayBreakout gameplayBreakout;

	Gpu gpu;
	gpu.Init();

	GpuBreakout gpuBreakout;
	gpuBreakout.Init(&gpu.device);
	gpuBreakout.Load();
	gpuBreakout.Setup();

	Input::Init();
	currentTime = timer.GetTime();
	lastTime = currentTime;
	while (Input::GetKeyInputState(Input::KEY_ESCAPE) != Input::INPUT_STATE_PRESSED)
	{
		currentTime = timer.GetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Input::Update();

		gameplayBreakout.Input(gpuBreakout.GetClientScreenSpace());
		gameplayBreakout.Update((float)deltaTime);

		gpuBreakout.Prepare(gameplayBreakout);

		gameplayBreakout.Render();

		gpuBreakout.Render();

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	gpuBreakout.ShutDown();
	gpu.ShutDown();

	system("pause");

	return 0;
}