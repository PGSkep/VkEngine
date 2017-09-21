#include "GpuController.h"

#define VIEW_PROJECTION_UNIFORM_BINDING 0
#define MODEL_MATRICES_UNIFORM_BINDING 1

#define DIFFUSE_COMBINED_IMAGE_SAMPLER_BINDING 0

#define STAGING_IMAGE_WIDTH_CAP 4096
#define STAGING_IMAGE_HEIGHT_CAP 4096

void GpuController::Init(VkS::Device* _device)
{
	timer.SetResolution(Timer::RESOLUTION::RESOLUTION_NANOSECONDS);
	timer.Play();
	lastTime = timer.GetTime();

	scene.Init();

	device = _device;
	window = device->windows[0];

	graphicsQueueGroupIndex = 0;
	graphicsQueue = device->queueGroups[graphicsQueueGroupIndex].queues[0];

	// setupFence
	{
		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkU::vkApiResult = vkCreateFence(device->handle, &fenceCreateInfo, nullptr, &setupFence);
	}

	// renderPass
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions(2);
		attachmentDescriptions[0].flags = 0;
		attachmentDescriptions[0].format = device->windows[0]->colorFormat.format;
		attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachmentDescriptions[1].flags = 0;
		attachmentDescriptions[1].format = device->physicalDevice->depthFormat;
		attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentReference;
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentReference;
		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpassDescription;
		subpassDescription.flags = VK_RESERVED_FOR_FUTURE_USE;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		VkSubpassDependency subpassDependency;
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = 0;
		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		VkU::vkApiResult = vkCreateRenderPass(device->handle, &renderPassCreateInfo, nullptr, &renderPass);
	}

	// GraphicsCommandPool
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = device->queueGroups[graphicsQueueGroupIndex].familyIndex;
		VkU::vkApiResult = vkCreateCommandPool(device->handle, &commandPoolCreateInfo, nullptr, &graphicsCommandPool);
	}

	// GraphicsCommandBuffer
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = graphicsCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;
		VkU::vkApiResult = vkAllocateCommandBuffers(device->handle, &commandBufferAllocateInfo, &graphicsCommandBuffer);
	}

	// Window Resources
	{
		VkU::CreateWindowResourcesInfo createWindowResourcesInfo;
		createWindowResourcesInfo.vkDevice = device->handle;
		createWindowResourcesInfo.physicalDevice = device->physicalDevice;
		createWindowResourcesInfo.setupFence = setupFence;
		createWindowResourcesInfo.setupCommandBuffer = graphicsCommandBuffer;
		createWindowResourcesInfo.setupQueue = graphicsQueue;
		createWindowResourcesInfo.graphicsCommandPool = graphicsCommandPool;
		createWindowResourcesInfo.imageCount = 3;
		createWindowResourcesInfo.renderPass = renderPass;

		VkA::CreateWindowResources(*window, createWindowResourcesInfo);
	}

	// DescriptorSetLayout
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(2);
		// view projection
		descriptorSetLayoutBindings[0].binding = VIEW_PROJECTION_UNIFORM_BINDING;
		descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBindings[0].descriptorCount = 1;
		descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
		// models
		descriptorSetLayoutBindings[1].binding = MODEL_MATRICES_UNIFORM_BINDING;
		descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBindings[1].descriptorCount = 1;
		descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

		descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)descriptorSetLayoutBindings.size();
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
		VkU::vkApiResult = vkCreateDescriptorSetLayout(device->handle, &descriptorSetLayoutCreateInfo, nullptr, &mvpDescriptorSetLayout);

		descriptorSetLayoutBindings.clear();
		descriptorSetLayoutBindings.resize(1);
		// diffuse
		descriptorSetLayoutBindings[0].binding = DIFFUSE_COMBINED_IMAGE_SAMPLER_BINDING;
		descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetLayoutBindings[0].descriptorCount = 1;
		descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

		descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)descriptorSetLayoutBindings.size();
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
		VkU::vkApiResult = vkCreateDescriptorSetLayout(device->handle, &descriptorSetLayoutCreateInfo, nullptr, &diffuseDescriptorSetLayout);
	}

	// PipelineLayout
	{
		std::vector<VkPushConstantRange> pushConstantRanges(2);
		// vertex 0~4
		pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRanges[0].offset = 0;
		pushConstantRanges[0].size = sizeof(float) * 4;
		// fragment 4~8
		pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRanges[1].offset = sizeof(float) * 4;
		pushConstantRanges[1].size = sizeof(float) * 4;

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { mvpDescriptorSetLayout, diffuseDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

		VkU::vkApiResult = vkCreatePipelineLayout(device->handle, &pipelineLayoutCreateInfo, nullptr, &mvpPush4Vert4FragPipelineLayout);
	}

	// Shader Modules
	{
		VkU::CreateShaderModuleInfo createShaderModuleInfo;
		createShaderModuleInfo.vkDevice = device->handle;
		createShaderModuleInfo.filename = "Shaders/vert.spv";
		VkA::CreateShaderModule(vertexShaderModule, createShaderModuleInfo);

		createShaderModuleInfo.filename = "Shaders/frag.spv";
		VkA::CreateShaderModule(fragmentShaderModule, createShaderModuleInfo);
	}

	// PipelineData
	{
		// Shader
		{
			pipelineData.shaderStageCreateInfos.resize(1);
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.resize(2);

			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pNext = nullptr;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].module = vertexShaderModule;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pName = "main";
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pSpecializationInfo = nullptr;

			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pNext = nullptr;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].module = fragmentShaderModule;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pName = "main";
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pSpecializationInfo = nullptr;
		}

		// Vertex Input
		{
			pipelineData.vertexInputBindingDescriptions.resize(1);
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions.resize(1);
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].binding = 0;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].stride = sizeof(float) * (3 + 2 + 3);
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			pipelineData.vertexInputAttributeDescriptions.resize(1);
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions.resize(3);
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].location = 0;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].binding = 0;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].offset = 0;

			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].location = 1;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].binding = 0;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].offset = sizeof(float) * 3;

			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].location = 2;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].binding = 0;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].offset = sizeof(float) * 5;

			pipelineData.pipelineVertexInputStateCreateInfos.resize(1);
			pipelineData.pipelineVertexInputStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			pipelineData.pipelineVertexInputStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineVertexInputStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineVertexInputStateCreateInfos[0].vertexBindingDescriptionCount = (uint32_t)pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions.size();
			pipelineData.pipelineVertexInputStateCreateInfos[0].pVertexBindingDescriptions = pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions.data();
			pipelineData.pipelineVertexInputStateCreateInfos[0].vertexAttributeDescriptionCount = (uint32_t)pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions.size();
			pipelineData.pipelineVertexInputStateCreateInfos[0].pVertexAttributeDescriptions = pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions.data();
		}

		// Input Assembly
		{
			pipelineData.pipelineInputAssemblyStateCreateInfos.resize(1);
			pipelineData.pipelineInputAssemblyStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			pipelineData.pipelineInputAssemblyStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineInputAssemblyStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineInputAssemblyStateCreateInfos[0].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipelineData.pipelineInputAssemblyStateCreateInfos[0].primitiveRestartEnable = VK_FALSE;
		}

		// Tessalation
		{
			pipelineData.pipelineTessellationStateCreateInfo.resize(1);
			pipelineData.pipelineTessellationStateCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			pipelineData.pipelineTessellationStateCreateInfo[0].pNext = nullptr;
			pipelineData.pipelineTessellationStateCreateInfo[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineTessellationStateCreateInfo[0].patchControlPoints = 3;
		}

		// Viewport
		{
			pipelineData.viewports.resize(1);
			pipelineData.viewports[0].viewports.resize(1);
			pipelineData.viewports[0].viewports[0].x = 0.0f;
			pipelineData.viewports[0].viewports[0].y = 0.0f;
			pipelineData.viewports[0].viewports[0].width = (float)device->windows[0]->extent.width;
			pipelineData.viewports[0].viewports[0].height = (float)device->windows[0]->extent.height;
			pipelineData.viewports[0].viewports[0].minDepth = 0.0f;
			pipelineData.viewports[0].viewports[0].maxDepth = 1.0f;

			pipelineData.scissors.resize(1);
			pipelineData.scissors[0].scissors.resize(1);
			pipelineData.scissors[0].scissors[0].offset = { 0, 0 };
			pipelineData.scissors[0].scissors[0].extent = device->windows[0]->extent;

			pipelineData.pipelineViewportStateCreateInfos.resize(1);
			pipelineData.pipelineViewportStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			pipelineData.pipelineViewportStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineViewportStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineViewportStateCreateInfos[0].viewportCount = (uint32_t)pipelineData.viewports[0].viewports.size();
			pipelineData.pipelineViewportStateCreateInfos[0].pViewports = pipelineData.viewports[0].viewports.data();
			pipelineData.pipelineViewportStateCreateInfos[0].scissorCount = (uint32_t)pipelineData.scissors[0].scissors.size();
			pipelineData.pipelineViewportStateCreateInfos[0].pScissors = pipelineData.scissors[0].scissors.data();
		}

		// Rasterization
		{
			pipelineData.pipelineRasterizationStateCreateInfos.resize(2);
			pipelineData.pipelineRasterizationStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			pipelineData.pipelineRasterizationStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineRasterizationStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].depthClampEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].rasterizerDiscardEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].polygonMode = VK_POLYGON_MODE_FILL;
			pipelineData.pipelineRasterizationStateCreateInfos[0].cullMode = VK_CULL_MODE_NONE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].frontFace = VK_FRONT_FACE_CLOCKWISE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].depthBiasEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[0].depthBiasConstantFactor = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[0].depthBiasClamp = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[0].depthBiasSlopeFactor = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[0].lineWidth = 1.0f;

			pipelineData.pipelineRasterizationStateCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			pipelineData.pipelineRasterizationStateCreateInfos[1].pNext = nullptr;
			pipelineData.pipelineRasterizationStateCreateInfos[1].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].depthClampEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].rasterizerDiscardEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].polygonMode = VK_POLYGON_MODE_LINE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].cullMode = VK_CULL_MODE_NONE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].frontFace = VK_FRONT_FACE_CLOCKWISE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].depthBiasEnable = VK_FALSE;
			pipelineData.pipelineRasterizationStateCreateInfos[1].depthBiasConstantFactor = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[1].depthBiasClamp = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[1].depthBiasSlopeFactor = 0.0f;
			pipelineData.pipelineRasterizationStateCreateInfos[1].lineWidth = 1.0f;
		}

		// Multisample
		{
			pipelineData.pipelineMultisampleStateCreateInfos.resize(1);
			pipelineData.pipelineMultisampleStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			pipelineData.pipelineMultisampleStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineMultisampleStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineMultisampleStateCreateInfos[0].rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineData.pipelineMultisampleStateCreateInfos[0].sampleShadingEnable = VK_FALSE;
			pipelineData.pipelineMultisampleStateCreateInfos[0].minSampleShading = 0.0f;
			pipelineData.pipelineMultisampleStateCreateInfos[0].pSampleMask = nullptr;
			pipelineData.pipelineMultisampleStateCreateInfos[0].alphaToCoverageEnable = VK_FALSE;
			pipelineData.pipelineMultisampleStateCreateInfos[0].alphaToOneEnable = VK_FALSE;
		}

		// Depth
		{
			pipelineData.pipelineDepthStencilStateCreateInfos.resize(1);
			pipelineData.pipelineDepthStencilStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthTestEnable = VK_TRUE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthWriteEnable = VK_TRUE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthBoundsTestEnable = VK_FALSE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].stencilTestEnable = VK_FALSE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].front = {};
			pipelineData.pipelineDepthStencilStateCreateInfos[0].back = {};
			pipelineData.pipelineDepthStencilStateCreateInfos[0].minDepthBounds = 0.0f;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].maxDepthBounds = 1.0f;
		}

		// Color Blend
		{
			pipelineData.pipelineColorBlendAttachmentStates.resize(1);
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates.resize(1);
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].blendEnable = VK_TRUE;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
			pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			pipelineData.pipelineColorBlendStateCreateInfos.resize(1);
			pipelineData.pipelineColorBlendStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			pipelineData.pipelineColorBlendStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineColorBlendStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineColorBlendStateCreateInfos[0].logicOpEnable = VK_FALSE;
			pipelineData.pipelineColorBlendStateCreateInfos[0].logicOp = VK_LOGIC_OP_CLEAR;
			pipelineData.pipelineColorBlendStateCreateInfos[0].attachmentCount = (uint32_t)pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates.size();
			pipelineData.pipelineColorBlendStateCreateInfos[0].pAttachments = pipelineData.pipelineColorBlendAttachmentStates[0].pipelineColorBlendAttachmentStates.data();
			pipelineData.pipelineColorBlendStateCreateInfos[0].blendConstants[0] = 0.0f;
			pipelineData.pipelineColorBlendStateCreateInfos[0].blendConstants[1] = 0.0f;
			pipelineData.pipelineColorBlendStateCreateInfos[0].blendConstants[2] = 0.0f;
			pipelineData.pipelineColorBlendStateCreateInfos[0].blendConstants[3] = 0.0f;
		}

		// Dynamic
		{}

		// GraphicsPipelineCreateInfos
		{
			graphicsPipelineCreateInfos.resize(2);
			graphicsPipelineCreateInfos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfos[0].pNext = nullptr;
			graphicsPipelineCreateInfos[0].flags = 0;
			graphicsPipelineCreateInfos[0].stageCount = (uint32_t)pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.size();
			graphicsPipelineCreateInfos[0].pStages = pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.data();
			graphicsPipelineCreateInfos[0].pVertexInputState = &pipelineData.pipelineVertexInputStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pInputAssemblyState = &pipelineData.pipelineInputAssemblyStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pTessellationState = &pipelineData.pipelineTessellationStateCreateInfo[0];
			graphicsPipelineCreateInfos[0].pViewportState = &pipelineData.pipelineViewportStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pRasterizationState = &pipelineData.pipelineRasterizationStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pMultisampleState = &pipelineData.pipelineMultisampleStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pDepthStencilState = &pipelineData.pipelineDepthStencilStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pColorBlendState = &pipelineData.pipelineColorBlendStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pDynamicState = nullptr;
			graphicsPipelineCreateInfos[0].layout = mvpPush4Vert4FragPipelineLayout;
			graphicsPipelineCreateInfos[0].renderPass = renderPass;
			graphicsPipelineCreateInfos[0].subpass = 0;
			graphicsPipelineCreateInfos[0].basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfos[0].basePipelineIndex = 0;

			graphicsPipelineCreateInfos[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfos[1].pNext = nullptr;
			graphicsPipelineCreateInfos[1].flags = 0;
			graphicsPipelineCreateInfos[1].stageCount = (uint32_t)pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.size();
			graphicsPipelineCreateInfos[1].pStages = pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.data();
			graphicsPipelineCreateInfos[1].pVertexInputState = &pipelineData.pipelineVertexInputStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pInputAssemblyState = &pipelineData.pipelineInputAssemblyStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pTessellationState = &pipelineData.pipelineTessellationStateCreateInfo[0];
			graphicsPipelineCreateInfos[1].pViewportState = &pipelineData.pipelineViewportStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pRasterizationState = &pipelineData.pipelineRasterizationStateCreateInfos[1];
			graphicsPipelineCreateInfos[1].pMultisampleState = &pipelineData.pipelineMultisampleStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pDepthStencilState = &pipelineData.pipelineDepthStencilStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pColorBlendState = &pipelineData.pipelineColorBlendStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pDynamicState = nullptr;
			graphicsPipelineCreateInfos[1].layout = mvpPush4Vert4FragPipelineLayout;
			graphicsPipelineCreateInfos[1].renderPass = renderPass;
			graphicsPipelineCreateInfos[1].subpass = 0;
			graphicsPipelineCreateInfos[1].basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfos[1].basePipelineIndex = 0;
		}
	}

	// GraphicsPipeline
	{}
	{
		graphicsPipelines.resize(graphicsPipelineCreateInfos.size());
		vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, (uint32_t)graphicsPipelineCreateInfos.size(), graphicsPipelineCreateInfos.data(), nullptr, graphicsPipelines.data());
	}

	// ComputePipeline
	{

	}

	// Buffers
	{
		VkU::CreateBufferInfo createBufferInfo;
		createBufferInfo.vkDevice = device->handle;
		createBufferInfo.physicalDevice = device->physicalDevice;

		// staging
		{
			createBufferInfo.size = sizeof(uint8_t) * (2 + 1024) * sizeof(Math3D::Mat4); // 2 mat4(view projection) 1024 mat4(model matrices)
			stagingBufferData = new uint8_t[createBufferInfo.size];

			createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			createBufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			VkA::CreateBuffer(stagingBuffer, createBufferInfo);
		}

		VkU::FillBufferInfo fillBufferInfo;
		fillBufferInfo.vkDevice = device->handle;
		fillBufferInfo.dstBuffer = stagingBuffer;

		VkU::CopyBufferInfo copyBufferInfo;
		copyBufferInfo.vkDevice = device->handle;
		copyBufferInfo.setupFence = setupFence;
		copyBufferInfo.setupQueue = graphicsQueue;
		copyBufferInfo.setupCommandBuffer = graphicsCommandBuffer;
		copyBufferInfo.srcBuffer = stagingBuffer.handle;
		copyBufferInfo.copyRegions.resize(1);
		copyBufferInfo.copyRegions[0].srcOffset = 0;
		copyBufferInfo.copyRegions[0].dstOffset = 0;

		// view projection
		{
			viewProjectionData[0] = scene.view;

			Math3D::Mat4 a = Math3D::GetPerspectiveMatrix(45.0f, (float)window->extent.width / (float)window->extent.height, 0.1f, 1000.0f);
			viewProjectionData[1] = Math3D2::Mat4::GetPerspectiveProjection(45.0f, (float)window->extent.width, (float)window->extent.height, 0.1f, 1000.0f);

			createBufferInfo.size = sizeof(viewProjectionData);
			createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			createBufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkA::CreateBuffer(viewProjectionBuffer, createBufferInfo);

			fillBufferInfo.offset = 0;
			fillBufferInfo.size = sizeof(viewProjectionData);
			fillBufferInfo.data = viewProjectionData;
			VkA::FillBuffer(fillBufferInfo);

			copyBufferInfo.copyRegions[0].size = viewProjectionBuffer.size;
			copyBufferInfo.dstBuffer = viewProjectionBuffer.handle;
			VkA::CopyBuffer(copyBufferInfo);
		}

		// model matrices
		{
			ZeroMemory(modelMatricesData, sizeof(modelMatricesData));

			createBufferInfo.size = sizeof(modelMatricesData);
			createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			createBufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkA::CreateBuffer(modelMatricesBuffer, createBufferInfo);

			fillBufferInfo.offset = 0;
			fillBufferInfo.size = sizeof(modelMatricesData);
			fillBufferInfo.data = modelMatricesData;
			VkA::FillBuffer(fillBufferInfo);

			copyBufferInfo.copyRegions[0].size = modelMatricesBuffer.size;
			copyBufferInfo.dstBuffer = modelMatricesBuffer.handle;
			VkA::CopyBuffer(copyBufferInfo);
		}

		// vertex buffer
		{
			float vertices[] = {
				//	Position					TexCoord			Normal
				+0.0f, -0.5f, -0.0f,		+0.0f, -1.0f,		0.0f, 1.0f, 0.0f,
				+0.5f, +0.5f, -0.0f,		+1.0f, +1.0f,		0.0f, 1.0f, 0.0f,
				-0.5f, +0.5f, -0.0f,		-1.0f, +1.0f,		0.0f, 1.0f, 0.0f, };

			createBufferInfo.size = sizeof(vertices);
			createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			createBufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkA::CreateBuffer(vertexBuffer, createBufferInfo);

			fillBufferInfo.offset = 0;
			fillBufferInfo.size = sizeof(vertices);
			fillBufferInfo.data = vertices;
			VkA::FillBuffer(fillBufferInfo);

			copyBufferInfo.copyRegions[0].size = vertexBuffer.size;
			copyBufferInfo.dstBuffer = vertexBuffer.handle;
			VkA::CopyBuffer(copyBufferInfo);
		}

		// index buffer
		{
			uint16_t indices[] = { 0, 1, 2 };

			createBufferInfo.size = sizeof(indices);
			createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			createBufferInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkA::CreateBuffer(indexBuffer, createBufferInfo);

			fillBufferInfo.offset = 0;
			fillBufferInfo.size = sizeof(indices);
			fillBufferInfo.data = indices;
			VkA::FillBuffer(fillBufferInfo);

			copyBufferInfo.copyRegions[0].size = indexBuffer.size;
			copyBufferInfo.dstBuffer = indexBuffer.handle;
			VkA::CopyBuffer(copyBufferInfo);
		}
	}

	// Texture
	{}
	{
		VkU::CreateTextureInfo createTextureInfo;
		createTextureInfo.vkDevice = device->handle;
		createTextureInfo.physicalDevice = device->physicalDevice;

		VkU::FillTextureInfo fillTextureInfo;
		fillTextureInfo.vkDevice = device->handle;

		VkU::CopyTextureInfo copyTextureInfo;
		copyTextureInfo.vkDevice = device->handle;
		copyTextureInfo.setupFence = setupFence;
		copyTextureInfo.setupCommandBuffer = graphicsCommandBuffer;
		copyTextureInfo.setupQueue = graphicsQueue;

		// Staging
		createTextureInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		createTextureInfo.extent3D = { STAGING_IMAGE_WIDTH_CAP, STAGING_IMAGE_HEIGHT_CAP, 1 };
		createTextureInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createTextureInfo.tiling = VK_IMAGE_TILING_LINEAR;
		createTextureInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createTextureInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		VkA::CreateTexture(stagingTextureR8G8B8A8, createTextureInfo);

		// Texture
		VkS::ImageData imageData;
		imageData.data = nullptr;
		VkU::LoadImageDataInfo loadImageDataInfo;

		loadImageDataInfo.fileName = "Textures/test.tga";
		VkA::LoadImageData(imageData, loadImageDataInfo);

		createTextureInfo.format = imageData.format;
		createTextureInfo.extent3D = { imageData.extent2D.width, imageData.extent2D.height, 1 };
		createTextureInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createTextureInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createTextureInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		createTextureInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		VkA::CreateTexture(texture, createTextureInfo);

		fillTextureInfo.dstTexture = stagingTextureR8G8B8A8;
		fillTextureInfo.width = imageData.extent2D.width;
		fillTextureInfo.height = imageData.extent2D.height;
		fillTextureInfo.size = imageData.size;
		fillTextureInfo.data = imageData.data;
		VkA::FillTexture(fillTextureInfo);
		delete[] imageData.data;

		copyTextureInfo.srcTexture = stagingTextureR8G8B8A8;
		copyTextureInfo.dstTexture = texture;
		VkA::CopyTexture(copyTextureInfo);
	}

	// Sampler
	{}
	{
		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		vkCreateSampler(device->handle, &samplerCreateInfo, nullptr, &sampler);
	}

	// DescriptorPool
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes(2);
		descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSizes[0].descriptorCount = 2;
		descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = 2;
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

		vkCreateDescriptorPool(device->handle, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	}

	// DescriptorSets
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			mvpDescriptorSetLayout,
			diffuseDescriptorSetLayout,
		};
		std::vector<VkDescriptorSet> descriptorSets = {
			mvpDescriptorSet,
			diffuseDescriptorSet,
		};

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)descriptorSetLayouts.size();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vkAllocateDescriptorSets(device->handle, &descriptorSetAllocateInfo, descriptorSets.data());

		mvpDescriptorSet = descriptorSets[0];
		diffuseDescriptorSet = descriptorSets[1];
	}

	// UpdateDescriptorSets
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSet(3);

		// View Projection
		VkDescriptorBufferInfo viewProjectionDescriptorBufferInfo;
		viewProjectionDescriptorBufferInfo.buffer = viewProjectionBuffer.handle;
		viewProjectionDescriptorBufferInfo.offset = 0;
		viewProjectionDescriptorBufferInfo.range = sizeof(viewProjectionData);

		writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[0].pNext = nullptr;
		writeDescriptorSet[0].dstSet = mvpDescriptorSet;
		writeDescriptorSet[0].dstBinding = VIEW_PROJECTION_UNIFORM_BINDING;
		writeDescriptorSet[0].dstArrayElement = 0;
		writeDescriptorSet[0].descriptorCount = 1;
		writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet[0].pImageInfo = nullptr;
		writeDescriptorSet[0].pBufferInfo = &viewProjectionDescriptorBufferInfo;
		writeDescriptorSet[0].pTexelBufferView = nullptr;

		// model matricas
		VkDescriptorBufferInfo modelMatricesDescriptorBufferInfo;
		modelMatricesDescriptorBufferInfo.buffer = modelMatricesBuffer.handle;
		modelMatricesDescriptorBufferInfo.offset = 0;
		modelMatricesDescriptorBufferInfo.range = sizeof(modelMatricesData);

		writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[1].pNext = nullptr;
		writeDescriptorSet[1].dstSet = mvpDescriptorSet;
		writeDescriptorSet[1].dstBinding = MODEL_MATRICES_UNIFORM_BINDING;
		writeDescriptorSet[1].dstArrayElement = 0;
		writeDescriptorSet[1].descriptorCount = 1;
		writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet[1].pImageInfo = nullptr;
		writeDescriptorSet[1].pBufferInfo = &modelMatricesDescriptorBufferInfo;
		writeDescriptorSet[1].pTexelBufferView = nullptr;

		// diffuse texture
		VkDescriptorImageInfo diffuseDescriptorImageInfo;
		diffuseDescriptorImageInfo.sampler = sampler;
		diffuseDescriptorImageInfo.imageView = texture.view;
		diffuseDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[2].pNext = nullptr;
		writeDescriptorSet[2].dstSet = diffuseDescriptorSet;
		writeDescriptorSet[2].dstBinding = DIFFUSE_COMBINED_IMAGE_SAMPLER_BINDING;
		writeDescriptorSet[2].dstArrayElement = 0;
		writeDescriptorSet[2].descriptorCount = 1;
		writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet[2].pImageInfo = &diffuseDescriptorImageInfo;
		writeDescriptorSet[2].pBufferInfo = nullptr;
		writeDescriptorSet[2].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device->handle, (uint32_t)writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);
	}
}
void GpuController::Run()
{
	double newTime = timer.GetTime();
	deltaTime = lastTime - newTime;
	lastTime = newTime;

	// Draw o secondary command buffer
	{
		//std::cout << "Aquire\n";
		vkWaitForFences(device->handle, 1, &setupFence, VK_TRUE, -1);
		vkResetFences(device->handle, 1, &setupFence);
		vkAcquireNextImageKHR(device->handle, window->swapchain, ~0U, window->imageAvailableSemaphore, setupFence, &window->imageIndex);

		VkCommandBufferInheritanceInfo commandBufferInheritanceInfo;
		commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		commandBufferInheritanceInfo.pNext = nullptr;
		commandBufferInheritanceInfo.renderPass = renderPass;
		commandBufferInheritanceInfo.subpass = 0;
		commandBufferInheritanceInfo.framebuffer = window->framebuffers[window->imageIndex];
		commandBufferInheritanceInfo.occlusionQueryEnable = VK_FALSE;
		commandBufferInheritanceInfo.queryFlags = 0;
		commandBufferInheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;

		//std::cout << "SecondaryBegin\n";
		//vkWaitForFences(device->handle, 1, &window->fences[window->imageIndex], VK_TRUE, -1);
		//vkResetFences(device->handle, 1, &window->fences[window->imageIndex]);

		vkBeginCommandBuffer(window->secondaryCommandBuffers[window->imageIndex], &commandBufferBeginInfo);

		VkDeviceSize offset = 0;

		struct VertexShaderPushConstantData
		{
			uint32_t modelMatrixIndex = 0;
			float red = 0.0f;
			float green = 0.0f;
			float blue = 0.0f;
		} vertexShaderPushConstantData;

		{
			vkCmdBindPipeline(window->secondaryCommandBuffers[window->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[0]);
			std::vector<VkDescriptorSet> descriptorSets = { mvpDescriptorSet, diffuseDescriptorSet };
			vkCmdBindDescriptorSets(window->secondaryCommandBuffers[window->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mvpPush4Vert4FragPipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);

			vkCmdPushConstants(window->secondaryCommandBuffers[window->imageIndex], mvpPush4Vert4FragPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexShaderPushConstantData), &vertexShaderPushConstantData);

			vkCmdBindVertexBuffers(window->secondaryCommandBuffers[window->imageIndex], 0, 1, &vertexBuffer.handle, &offset);
			vkCmdBindIndexBuffer(window->secondaryCommandBuffers[window->imageIndex], indexBuffer.handle, 0, VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(window->secondaryCommandBuffers[window->imageIndex], 6, 1, 0, 0, 0);
		}

		vkEndCommandBuffer(window->secondaryCommandBuffers[window->imageIndex]);
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkWaitForFences(device->handle, 1, &setupFence, VK_TRUE, -1);
	vkResetFences(device->handle, 1, &setupFence);

	vkBeginCommandBuffer(graphicsCommandBuffer, &commandBufferBeginInfo);

	// Update camera
	{
		if (GetAsyncKeyState('A'))
			scene.cameraPosition.x -= (float)(deltaTime);
		if (GetAsyncKeyState('D'))
			scene.cameraPosition.x += (float)(deltaTime);

		if (GetAsyncKeyState('Q'))
			scene.cameraPosition.y -= (float)(deltaTime);
		if (GetAsyncKeyState('E'))
			scene.cameraPosition.y += (float)(deltaTime);

		if (GetAsyncKeyState('W'))
			scene.cameraPosition.z -= (float)(deltaTime);
		if (GetAsyncKeyState('S'))
			scene.cameraPosition.z += (float)(deltaTime);

		scene.view = Math3D2::Mat4::GetLookAt(scene.cameraPosition, scene.cameraTarget, scene.cameraUp);

		viewProjectionData[0] = scene.view;
	}

	// Update model
	{
		static Math3D2::Mat4 transform = Math3D2::Mat4::GetIdentity();
		
		if (!GetAsyncKeyState(VK_SHIFT))
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ (float)(-deltaTime), 0.0f, 0.0f }) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ (float)(+deltaTime), 0.0f, 0.0f }) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ 0.0f, (float)(-deltaTime), 0.0f }) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ 0.0f, (float)(+deltaTime), 0.0f }) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, (float)(-deltaTime) }) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D2::Mat4::GetTranslateMatrix({ 0.0f, 0.0f, (float)(+deltaTime) }) * transform;
		}
		else
		{
			if (GetAsyncKeyState('J'))
				transform = Math3D2::Mat4::GetRotateXMatrix((float)(-deltaTime)) * transform;
			if (GetAsyncKeyState('L'))
				transform = Math3D2::Mat4::GetRotateXMatrix((float)(+deltaTime)) * transform;

			if (GetAsyncKeyState('U'))
				transform = Math3D2::Mat4::GetRotateYMatrix((float)(-deltaTime)) * transform;
			if (GetAsyncKeyState('O'))
				transform = Math3D2::Mat4::GetRotateYMatrix((float)(+deltaTime)) * transform;

			if (GetAsyncKeyState('I'))
				transform = Math3D2::Mat4::GetRotateZMatrix((float)(-deltaTime)) * transform;
			if (GetAsyncKeyState('K'))
				transform = Math3D2::Mat4::GetRotateZMatrix((float)(+deltaTime)) * transform;
		}

		modelMatricesData[0] = transform;
	}

	// Fill Staging
	{
		VkU::FillBufferInfo2 fillBufferInfo2;
		fillBufferInfo2.vkDevice = device->handle;
		fillBufferInfo2.dstBuffer = stagingBuffer;
		fillBufferInfo2.targetBufferMemoryOffset = 0;
		fillBufferInfo2.targetBufferMemorySize = stagingBuffer.size;
		fillBufferInfo2.memoryFillRegions.resize(2);

		fillBufferInfo2.memoryFillRegions[0].targetBufferMemoryOffset = 0;
		fillBufferInfo2.memoryFillRegions[0].size = sizeof(viewProjectionData);
		fillBufferInfo2.memoryFillRegions[0].data = viewProjectionData;

		fillBufferInfo2.memoryFillRegions[1].targetBufferMemoryOffset = sizeof(viewProjectionData);
		fillBufferInfo2.memoryFillRegions[1].size = sizeof(modelMatricesData);
		fillBufferInfo2.memoryFillRegions[1].data = modelMatricesData;
		VkA::FillBuffer2(fillBufferInfo2);
	}

	// Copy from staging to device buffers
	{
		VkU::CopyBuffersInfo copyBuffersInfo;
		copyBuffersInfo.commandBuffer = graphicsCommandBuffer;
		copyBuffersInfo.srcBuffer = stagingBuffer.handle;

		copyBuffersInfo.bufferRegion.resize(2);
		copyBuffersInfo.bufferRegion[0].dstBuffer = viewProjectionBuffer.handle;
		copyBuffersInfo.bufferRegion[0].copyRegions.resize(1);
		copyBuffersInfo.bufferRegion[0].copyRegions[0].srcOffset = 0;
		copyBuffersInfo.bufferRegion[0].copyRegions[0].dstOffset = 0;
		copyBuffersInfo.bufferRegion[0].copyRegions[0].size = sizeof(viewProjectionData);

		copyBuffersInfo.bufferRegion[1].dstBuffer = modelMatricesBuffer.handle;
		copyBuffersInfo.bufferRegion[1].copyRegions.resize(1);
		copyBuffersInfo.bufferRegion[1].copyRegions[0].srcOffset = sizeof(viewProjectionData);
		copyBuffersInfo.bufferRegion[1].copyRegions[0].dstOffset = 0;
		copyBuffersInfo.bufferRegion[1].copyRegions[0].size = sizeof(modelMatricesData);
		VkA::CopyBuffers(copyBuffersInfo);
	}

	VkClearValue clearColor[2];
	clearColor[0].color = { 0.3f, 0.3f, 0.5f, 0.0f };
	clearColor[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = window->framebuffers[window->imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { window->extent.width, window->extent.height };
	renderPassBeginInfo.clearValueCount = sizeof(clearColor) / sizeof(VkClearValue);
	renderPassBeginInfo.pClearValues = clearColor;
	vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		vkCmdExecuteCommands(graphicsCommandBuffer, 1, &window->secondaryCommandBuffers[window->imageIndex]);

	vkCmdEndRenderPass(graphicsCommandBuffer);

	vkEndCommandBuffer(graphicsCommandBuffer);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::vector<VkSemaphore> waitSemaphores = { window->imageAvailableSemaphore };

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &graphicsCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &window->renderDoneSemaphore;
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, setupFence);
	
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &window->renderDoneSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window->swapchain;
	presentInfo.pImageIndices = &window->imageIndex;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(graphicsQueue, &presentInfo);

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
void GpuController::ShutDown()
{
	std::vector<VkS::Window*> windows = { window };

	vkWaitForFences(device->handle, 1, &setupFence, VK_TRUE, ~0U);
	for (size_t iWindow = 0; iWindow != windows.size(); ++iWindow)
		vkWaitForFences(device->handle, (uint32_t)windows[iWindow]->fences.size(), windows[iWindow]->fences.data(), VK_TRUE, ~0U);

	vkDeviceWaitIdle(device->handle);

	/// Gpu controller stuff
	// Descriptor Pool
	vkDestroyDescriptorPool(device->handle, descriptorPool, nullptr);

	// Sampler
	vkDestroySampler(device->handle, sampler, nullptr);

	// Textures
	vkDestroyImageView(device->handle, stagingTextureR8G8B8A8.view, nullptr);
	vkDestroyImage(device->handle, stagingTextureR8G8B8A8.handle, nullptr);
	vkFreeMemory(device->handle, stagingTextureR8G8B8A8.memory, nullptr);

	vkDestroyImageView(device->handle, texture.view, nullptr);
	vkDestroyImage(device->handle, texture.handle, nullptr);
	vkFreeMemory(device->handle, texture.memory, nullptr);

	// Buffers
	vkDestroyBuffer(device->handle, indexBuffer.handle, nullptr);
	vkFreeMemory(device->handle, indexBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, vertexBuffer.handle, nullptr);
	vkFreeMemory(device->handle, vertexBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, modelMatricesBuffer.handle, nullptr);
	vkFreeMemory(device->handle, modelMatricesBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, viewProjectionBuffer.handle, nullptr);
	vkFreeMemory(device->handle, viewProjectionBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, stagingBuffer.handle, nullptr);
	vkFreeMemory(device->handle, stagingBuffer.memory, nullptr);
	delete[] stagingBufferData;

	// Pipeline
	for (size_t iPipeline = 0; iPipeline != graphicsPipelines.size(); ++iPipeline)
		vkDestroyPipeline(device->handle, graphicsPipelines[iPipeline], nullptr);

	// Shader Module
	vkDestroyShaderModule(device->handle, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(device->handle, vertexShaderModule, nullptr);

	// PipelineLayout
	vkDestroyPipelineLayout(device->handle, mvpPush4Vert4FragPipelineLayout, nullptr);

	// DescriptorSetLayout
	vkDestroyDescriptorSetLayout(device->handle, diffuseDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->handle, mvpDescriptorSetLayout, nullptr);

	/// Window Resources
	for (size_t iWindow = 0; iWindow != windows.size(); ++iWindow)
	{
		vkDestroySemaphore(device->handle, windows[iWindow]->renderDoneSemaphore, nullptr);
		vkDestroySemaphore(device->handle, windows[iWindow]->imageAvailableSemaphore, nullptr);

		for (size_t iFence = 0; iFence != windows[iWindow]->fences.size(); ++iFence)
			vkDestroyFence(device->handle, windows[iWindow]->fences[iFence], nullptr);

		for (size_t iFramebuffer = 0; iFramebuffer != windows[iWindow]->framebuffers.size(); ++iFramebuffer)
			vkDestroyFramebuffer(device->handle, windows[iWindow]->framebuffers[iFramebuffer], nullptr);

		if (windows[iWindow]->depthImage != nullptr)
		{
			vkDestroyImage(device->handle, windows[iWindow]->depthImage->handle, nullptr);
			vkFreeMemory(device->handle, windows[iWindow]->depthImage->memory, nullptr);
			vkDestroyImageView(device->handle, windows[iWindow]->depthImage->view, nullptr);

			delete windows[iWindow]->depthImage;
		}

		for (size_t iView = 0; iView != windows[iWindow]->views.size(); ++iView)
			vkDestroyImageView(device->handle, windows[iWindow]->views[iView], nullptr);

		vkDestroySwapchainKHR(device->handle, windows[iWindow]->swapchain, nullptr);
	}
	// RenderPass
	vkDestroyRenderPass(device->handle, renderPass, nullptr);

	// CommandPool
	vkDestroyCommandPool(device->handle, graphicsCommandPool, nullptr);

	// Fence
	vkDestroyFence(device->handle, setupFence, nullptr);
}
