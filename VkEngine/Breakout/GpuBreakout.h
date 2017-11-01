#ifndef	GPU_BREAKOUT_H
#define GPU_BREAKOUT_H

#include "../_Common/Vulkan/vku.h"
#include "../_Common/DataPack.h"
#include "../_Common/Math3D.h"

#include "GameplayBreakout.h"

class GpuBreakout
{

	/// Init
	VkU::Device* device;
	VkU::Window* window;

	uint32_t graphicsQueueGroupIndex;
	VkQueue graphicsQueue;

	VkFence setupFence;

	VkRenderPass renderPass;

	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsCommandBuffer;

	// Samplers
	VkSampler nearestSampler;
	VkSampler linearSampler;

	// Layouts
	VkDescriptorSetLayout diffuseDescriptorSetLayout;

	VkPipelineLayout diffuseVert4PipelineLayout;

	/// Load
	// Shaders
	std::vector<VkShaderModule> shaderModules;

	VkShaderModule vertexSprite2DShaderModule;
	VkShaderModule fragmentSprite2DShaderModule;

	// Pipelines
	VkU::PipelineData pipelineData;
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;
	std::vector<VkPipeline> graphicsPipelines;

	VkPipeline sprite2DPipeline;

	// Textures
	VkU::Texture diffuseTexture;

	// Buffers
	std::vector<Math3D::Vec4> instanceData;
	VkU::Buffer modelMatricesBuffer;
	VkU::Buffer modelMatricesStagingBuffer;

	std::vector<VkU::VertexAndIndexOffsetAndSize> offsetsAndSizes;
	VkU::Buffer vertexBuffer;
	VkU::Buffer indexBuffer;

	/// Setup
	VkDescriptorPool descriptorPool;

	// Descriptors
	VkDescriptorSet diffuseDescriptorSet;

public:
	void Init(VkU::Device* _device);
	void Load();
	void Setup();

	void Prepare(GameplayBreakout& _scene);
	void Render();

	void ShutDown();

	RECT GetClientScreenSpace();
};

#endif