#ifndef	GPU_CONTROLLER_H
#define GPU_CONTROLLER_H

#include "Vk.h"

class GpuController
{
	VkS::Device* device;
	VkS::Window* window;
	VkS::Window* wireframeWindow;

	uint32_t graphicsQueueGroupIndex;
	VkQueue graphicsQueue;
	uint32_t computeQueueGroupIndex;

	VkFence setupFence;

	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsSetupCommandBuffer;

	VkRenderPass renderPass;

	VkDescriptorSetLayout mvpDescriptorSetLayout;
	VkDescriptorSetLayout diffuseDescriptorSetLayout;

	VkPipelineLayout mvpPush4Vert4FragPipelineLayout;

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
	//VkShaderModule tessalationControlShaderModule;
	//VkShaderModule tessalationEvaluationShaderModule;

	VkS::PipelineData pipelineData;
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;

	std::vector<VkPipeline> graphicsPipelines;
	VkPipeline computePipeline;

	void* stagingBufferData;
	VkS::Buffer stagingBuffer;
	glm::mat4 viewProjectionData[2];
	VkS::Buffer viewProjectionBuffer;
	glm::mat4 modelMatricesData[1024];
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
	void Draw(VkS::Window* _window, uint32_t _pipelineIndex);
	void Run();
	void ShutDown();
};

#endif