#ifndef	GPU_CONTROLLER_H
#define GPU_CONTROLLER_H

#include "VkU.h"
#include "Math3D.h"
#include "Scene.h"

class GpuController
{
	// Init
	VkU::Device* device;
	VkU::Window* window;

	uint32_t graphicsQueueGroupIndex;
	VkQueue graphicsQueue;

	VkFence setupFence;

	VkRenderPass renderPass;

	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsCommandBuffer;

	VkDescriptorSetLayout mvpDescriptorSetLayout;
	VkDescriptorSetLayout diffuseNormalDescriptorSetLayout;
	VkDescriptorSetLayout fontDescriptorSetLayout;

	VkPipelineLayout fontPush16Vert16Frag16GeomPipelineLayout;
	VkPipelineLayout mvpDiffuseNormalPush8Vert8FragPipelineLayout;

	VkSampler nearestSampler;
	VkSampler linearSampler;

	// Load
	VkShaderModule vertexTextShaderModule;
	VkShaderModule geometryTextShaderModule;
	VkShaderModule fragmentTextShaderModule;
	VkShaderModule vertexParallaxOcclusionShaderModule;
	VkShaderModule fragmentParallaxOcclusionShaderModule;

	VkU::PipelineData pipelineData;
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;
	std::vector<VkPipeline> graphicsPipelines;
	VkPipeline textPipeline;
	VkPipeline parallaxOcclusionPipeline;

	VkU::Texture fontTexture;
	VkU::Texture rocksDiffuseTexture;
	VkU::Texture rocksNormalTexture;

	VkU::Buffer viewProjectionBuffer;
	VkU::Buffer viewProjectionStagingBuffer;
	VkU::Buffer modelMatricesBuffer;
	VkU::Buffer modelMatricesStagingBuffer;
	VkU::Buffer vertexBuffer;
	VkU::Buffer indexBuffer;

	// Setup
	VkDescriptorPool descriptorPool;

	VkDescriptorSet fontDescriptorSet;
	VkDescriptorSet mvpDescriptorSet;
	VkDescriptorSet diffuseNormalDescriptorSet;

public:
	void Init(VkU::Device* _device);
	void Load(Scene& _scene);
	void Setup(Scene& _scene);

	void Compute() {};
	void Render(Scene& _scene, float _time, float _deltaTime);

	void ShutDown();
};

#endif