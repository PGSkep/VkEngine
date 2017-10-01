#define _CRTDBG_MAP_ALLOC // memory leak detection
#include<crtdbg.h> // memory leak detection
#include <stdlib.h> // system

#include "Gpu.h"
#include "GpuController.h"

#include "Math3D.h"
#include "Input.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	Gpu gpu;
	gpu.Init();

	GpuController gpuController;
	gpuController.Init(&gpu.device);
	
	while(!GetAsyncKeyState(VK_ESCAPE))
		gpuController.Run();
	
	gpuController.ShutDown();

	gpu.ShutDown();

	system("pause");

	return 0;
}
