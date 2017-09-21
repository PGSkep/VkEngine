#ifndef	GPU_CONTROLLER_H
#define GPU_CONTROLLER_H

#include "Vk.h"

#include "Scene.h"
#include "Timer.h"

class GpuController
{
	Timer timer;
	double lastTime;
	double deltaTime;

	Scene scene;

	VkS::Device* device;
	VkS::Window* window;
	//VkS::Window* wireframeWindow;

	uint32_t graphicsQueueGroupIndex;
	VkQueue graphicsQueue;
	//uint32_t computeQueueGroupIndex;

	VkFence setupFence;

	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsCommandBuffer;

	VkRenderPass renderPass;

	VkDescriptorSetLayout mvpDescriptorSetLayout;
	VkDescriptorSetLayout diffuseDescriptorSetLayout;

	VkPipelineLayout mvpPush4Vert4FragPipelineLayout;

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	VkS::PipelineData pipelineData;
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;

	std::vector<VkPipeline> graphicsPipelines;
	//VkPipeline computePipeline;

	void* stagingBufferData;
	VkS::Buffer stagingBuffer;
	Math3D2::Mat4 viewProjectionData[2];
	VkS::Buffer viewProjectionBuffer;
	Math3D2::Mat4 modelMatricesData[1024];
	VkS::Buffer modelMatricesBuffer;
	VkS::Buffer vertexBuffer;
	VkS::Buffer indexBuffer;
	
	VkS::Texture stagingTextureR8G8B8A8;
	VkS::Texture texture;
	
	VkSampler sampler;
	
	VkDescriptorPool descriptorPool;
	
	VkDescriptorSet mvpDescriptorSet;
	VkDescriptorSet diffuseDescriptorSet;

public:
	void Init(VkS::Device* _device);
	void Run();
	void ShutDown();
};

#endif