#ifndef	GPU_H
#define GPU_H

#include "Vk.h"

class Gpu
{
public:
	VkS::Instance instance;
	std::vector<VkS::Window> windows;
	VkS::Device device;

	void Init();
	void ShutDown();
};

#endif