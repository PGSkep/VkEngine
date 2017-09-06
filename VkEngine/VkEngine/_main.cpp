#define _CRTDBG_MAP_ALLOC // memory leak detection
#include<crtdbg.h> // memory leak detection
#include <stdlib.h> // system

#include "Gpu.h"
#include "GpuController.h"

#include "Math3D.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	Math3D::Mat4 matA = Math3D::GetTranslateMatrix({ 1.0f, 2.0f, 3.0f });

	Math3D::Mat4 matB = Math3D::GetScaleMatrix({ 5.0f, 5.0f, 5.0f });;

	Math3D::Mat4 matC = matA * matB;
	Math3D::Mat4 matD = matB * matA;

	Math3D::Mat4 matE = Math3D::GetPerspectiveMatrix(45.0f, (800.0f / 600.0f), 0.1f, 1000.0f);

	Math3D::Mat4 matF = Math3D::GetIdentity();

	Math3D::Mat4 matG = Math3D::GetLookAt({ 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0 });

	float b = Math3D::Dot({ 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 });
	Math3D::Vec3 vB = Math3D::Cross({ 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 });
	Math3D::Vec3 vD = Math3D::Normalize({ 4.0, 5.0, 6.0 });

	//return 0;

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