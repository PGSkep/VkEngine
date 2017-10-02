#ifndef	GPU_CONTROLLER_H
#define GPU_CONTROLLER_H

#include "Vk.h"

#include "Scene.h"
#include "Timer.h"
#include "Loader.h"
#include "Input.h"

class GpuController
{
	uint32_t indexCount;

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
	VkDescriptorSetLayout diffuseNormalDescriptorSetLayout;
	VkDescriptorSetLayout fontDescriptorSetLayout;

	VkPipelineLayout mvpPush8Vert8FragPipelineLayout;
	VkPipelineLayout push16Vert16Frag16GeomPipelineLayout;

	VkShaderModule vertexTextShaderModule;
	VkShaderModule geometryTextShaderModule;
	VkShaderModule fragmentTextShaderModule;
	VkShaderModule vertexParallaxOcclusionShaderModule;
	VkShaderModule fragmentParallaxOcclusionShaderModule;

	VkS::PipelineData pipelineData;
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;

	std::vector<VkPipeline> graphicsPipelines;
	//VkPipeline computePipeline;

	VkPipeline textPipeline;
	VkPipeline parallaxOcclusionPipeline;

	void* stagingBufferData;
	VkS::Buffer stagingBuffer;
	Math3D::Mat4 viewProjectionData[2];
	VkS::Buffer viewProjectionBuffer;
	Math3D::Mat4 modelMatricesData[1024];
	VkS::Buffer modelMatricesBuffer;
	VkS::Buffer vertexBuffer;
	VkS::Buffer indexBuffer;

	VkS::Texture fontTexture;
	VkS::Texture rocksDiffuseTexture;
	VkS::Texture rocksNormalTexture;

	VkSampler nearestSampler;
	VkSampler linearSampler;
	
	VkDescriptorPool descriptorPool;
	
	VkDescriptorSet mvpDescriptorSet;
	VkDescriptorSet diffuseNormalDescriptorSet;
	VkDescriptorSet fontDescriptorSet;

public:
	void Init(VkS::Device* _device);
	void Run();
	void ShutDown();
};

#endif