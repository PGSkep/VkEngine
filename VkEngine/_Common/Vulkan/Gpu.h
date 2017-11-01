#ifndef	GPU_H
#define GPU_H

#include "VkU.h"

class Gpu
{
public:
	VkU::Instance instance;
	std::vector<VkU::Window> windows;
	VkU::Device device;

	void Init();
	void ShutDown();
};

#endif