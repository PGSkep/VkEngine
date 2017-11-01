#include "GpuBreakout.h"

//#define VIEW_PROJECTION_UNIFORM_BINDING 0
//#define MODEL_MATRICES_UNIFORM_BINDING 1

#define DIFFUSE_COMBINED_IMAGE_SAMPLER_BINDING 0
//#define NORMAL_COMBINED_IMAGE_SAMPLER_BINDING 1

//#define FONT_COMBINED_IMAGE_SAMPLER_BINDING 0

// new
#define   VERTEX_BUFFER_BINDING 0
#define INSTANCE_BUFFER_BINDING 1

#define SQUARE_MESH_ID 0
#define BALL_MESH_ID 1

#define INSTANCE_DATA_SIZE sizeof(float) * 4

void GpuBreakout::Init(VkU::Device * _device)
{
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

		VkU::CreateWindowResources(*window, createWindowResourcesInfo);
	}

	// Sampler
	{
		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
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

		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		vkCreateSampler(device->handle, &samplerCreateInfo, nullptr, &nearestSampler);

		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		vkCreateSampler(device->handle, &samplerCreateInfo, nullptr, &linearSampler);
	}

	// DescriptorSetLayout
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(1);
		descriptorSetLayoutBindings[0].binding = 0;
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
		// Sprite 2D
		std::vector<VkPushConstantRange> pushConstantRanges(1);
		pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRanges[0].offset = 0;
		pushConstantRanges[0].size = sizeof(float);
		
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { diffuseDescriptorSetLayout };
		
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		
		VkU::vkApiResult = vkCreatePipelineLayout(device->handle, &pipelineLayoutCreateInfo, nullptr, &diffuseVert4PipelineLayout);
	}
}
void GpuBreakout::Load()
{
	instanceData.resize(100);
	
	for (size_t iIndiceData = 0; iIndiceData != instanceData.size(); ++iIndiceData)
	{
		instanceData[iIndiceData] = { 0.0f, 0.0f, 0.5f, 0.5f };
	}

	DataPack dataPack;
	dataPack.textures = {
		DataPack::TextureInfo::GetTextureInfo("../_Data/Textures/rocks diffuse.tga"),
		//DataPack::TextureInfo::GetTextureInfo("../_Data/Textures/rocks normal.tga"),
		//DataPack::TextureInfo::GetTextureInfo("../_Data/Textures/font3.tga"),
	};
	dataPack.meshes = {
		DataPack::MeshInfo::GetMeshInfo("../_Data/Models/Square.fbx", (Definitions::VERTEX_DATATYPE)(Definitions::VDT_X | Definitions::VDT_Y | Definitions::VDT_UV)),
		DataPack::MeshInfo::GetMeshInfo("../_Data/Models/Circle.fbx", (Definitions::VERTEX_DATATYPE)(Definitions::VDT_X | Definitions::VDT_Y | Definitions::VDT_UV)),
	};

	// Shader Modules
	{
		VkU::CreateShaderModuleInfo createShaderModuleInfo;
		createShaderModuleInfo.vkDevice = device->handle;

		shaderModules.resize(2);

		// Sprite 2D
		createShaderModuleInfo.filename = "../_Data/Shaders/vert.spv";
		VkU::CreateShaderModule(shaderModules[0], createShaderModuleInfo);
		vertexSprite2DShaderModule = shaderModules[0];
		createShaderModuleInfo.filename = "../_Data/Shaders/frag.spv";
		VkU::CreateShaderModule(shaderModules[1], createShaderModuleInfo);
		fragmentSprite2DShaderModule = shaderModules[1];
	}

	// PipelineData
	{
		// Shader
		{
			pipelineData.shaderStageCreateInfos.resize(1);

			// Sprite 2D
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo.resize(2);
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pNext = nullptr;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].module = vertexSprite2DShaderModule;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pName = "main";
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[0].pSpecializationInfo = nullptr;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pNext = nullptr;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].module = fragmentSprite2DShaderModule;
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pName = "main";
			pipelineData.shaderStageCreateInfos[0].shaderStageCreateInfo[1].pSpecializationInfo = nullptr;
		}

		// Vertex Input
		{
			pipelineData.vertexInputBindingDescriptions.resize(1);

			// Sprite 2D
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions.resize(2);
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].binding = 0;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].stride = sizeof(float) * 4;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[1].binding = 1;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[1].stride = sizeof(float) * 4;
			pipelineData.vertexInputBindingDescriptions[0].vertexInputBindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			pipelineData.vertexInputAttributeDescriptions.resize(1);

			// Sprite 2D
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions.resize(4);
			// vertex position
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].location = 0;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].binding = VERTEX_BUFFER_BINDING;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[0].offset = 0;
			// vertex tex coord
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].location = 1;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].binding = VERTEX_BUFFER_BINDING;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[1].offset = sizeof(float) * 2;
			// instance position
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].location = 2;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].binding = INSTANCE_BUFFER_BINDING;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[2].offset = 0;
			// instance scale
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[3].location = 3;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[3].binding = INSTANCE_BUFFER_BINDING;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			pipelineData.vertexInputAttributeDescriptions[0].vertexInputAttributeDescriptions[3].offset = sizeof(float) * 2;
			
			pipelineData.pipelineVertexInputStateCreateInfos.resize(1);

			// Sprite 2D
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

			// Sprite 2D
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
			// Sprite 2D
			pipelineData.pipelineRasterizationStateCreateInfos.resize(1);

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

			// Sprite 2D
			pipelineData.pipelineDepthStencilStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].pNext = nullptr;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthTestEnable = VK_FALSE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthWriteEnable = VK_FALSE;
			pipelineData.pipelineDepthStencilStateCreateInfos[0].depthCompareOp = VK_COMPARE_OP_NEVER;
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

		// GraphicsPipelineCreateInfos
		{
			graphicsPipelineCreateInfos.resize(1);

			// Sprite 2D
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
			graphicsPipelineCreateInfos[0].layout = diffuseVert4PipelineLayout;
			graphicsPipelineCreateInfos[0].renderPass = renderPass;
			graphicsPipelineCreateInfos[0].subpass = 0;
			graphicsPipelineCreateInfos[0].basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfos[0].basePipelineIndex = 0;
		}
	}

	// GraphicsPipeline
	{}
	{
		graphicsPipelines.resize(graphicsPipelineCreateInfos.size());
		vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, (uint32_t)graphicsPipelineCreateInfos.size(), graphicsPipelineCreateInfos.data(), nullptr, graphicsPipelines.data());

		sprite2DPipeline = graphicsPipelines[0];
	}

	// ComputePipeline
	{

	}

	// Textures
	{
		Loader::TextureData textureData;

		VkU::CreateTextureInfo createTextureInfo;
		createTextureInfo.textureData = &textureData;
		createTextureInfo.vkDevice = device->handle;
		createTextureInfo.physicalDevice = device->physicalDevice;
		createTextureInfo.setupFence = setupFence;
		createTextureInfo.setupCommandBuffer = graphicsCommandBuffer;
		createTextureInfo.setupQueue = graphicsQueue;

		// Sprite 2D
		textureData.LoadTGA(dataPack.textures[0].filename.c_str());
		VkU::CreateTexture(diffuseTexture, createTextureInfo);
	}

	// Buffers
	{
		std::vector<Loader::ModelData> modelDatas(dataPack.meshes.size());
		offsetsAndSizes.resize(dataPack.meshes.size());
		VkDeviceSize vertexOffset = 0;
		VkDeviceSize indexOffset = 0;
		for (size_t iMesh = 0; iMesh != dataPack.meshes.size(); ++iMesh)
		{
			modelDatas[iMesh].LoadModel(dataPack.meshes[iMesh].filename.c_str(), dataPack.meshes[iMesh].datatype, true);
			offsetsAndSizes[iMesh] = VkU::VertexAndIndexOffsetAndSize::GetVertexAndIndexOffsetAndSize(vertexOffset, VK_INDEX_TYPE_UINT16, indexOffset, modelDatas[iMesh].indexDataCount);
			vertexOffset += modelDatas[iMesh].vertexDataSize;
			indexOffset += modelDatas[iMesh].indexDataSize;
		}

		VkU::CreateBufferInfo createBufferInfo;
		createBufferInfo.vkDevice = device->handle;
		createBufferInfo.physicalDevice = device->physicalDevice;
		createBufferInfo.setupFence = setupFence;
		createBufferInfo.setupCommandBuffer = graphicsCommandBuffer;
		createBufferInfo.setupQueue = graphicsQueue;

		// model matrices
		createBufferInfo.datas = { VkU::Data::GetData(nullptr, sizeof(Math3D::Vec4) * instanceData.size()) };
		createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkU::CreateBuffer(modelMatricesBuffer, &modelMatricesStagingBuffer, createBufferInfo);

		createBufferInfo.datas.resize(modelDatas.size());

		// vertex buffer
		for (size_t iMesh = 0; iMesh != modelDatas.size(); ++iMesh)
			createBufferInfo.datas[iMesh] = VkU::Data::GetData(modelDatas[iMesh].vertexData, modelDatas[iMesh].vertexDataSize);
		createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkU::CreateBuffer(vertexBuffer, nullptr, createBufferInfo);

		// index buffer
		for (size_t iMesh = 0; iMesh != modelDatas.size(); ++iMesh)
			createBufferInfo.datas[iMesh] = VkU::Data::GetData(modelDatas[iMesh].indexData, modelDatas[iMesh].indexDataSize);
		createBufferInfo.bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkU::CreateBuffer(indexBuffer, nullptr, createBufferInfo);
	}
}
void GpuBreakout::Setup()
{
	// DescriptorPool
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes(2);
		descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSizes[0].descriptorCount = 0;
		descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = 1;
		descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

		vkCreateDescriptorPool(device->handle, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	}

	// DescriptorSets
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			diffuseDescriptorSetLayout,
		};
		std::vector<VkDescriptorSet> descriptorSets = {
			diffuseDescriptorSet,
		};

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)descriptorSetLayouts.size();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vkAllocateDescriptorSets(device->handle, &descriptorSetAllocateInfo, descriptorSets.data());

		diffuseDescriptorSet = descriptorSets[0];
	}

	// UpdateDescriptorSets
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSet(1);

		VkDescriptorImageInfo diffuseDescriptorImageInfo;
		diffuseDescriptorImageInfo.sampler = nearestSampler;
		diffuseDescriptorImageInfo.imageView = diffuseTexture.view;
		diffuseDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[0].pNext = nullptr;
		writeDescriptorSet[0].dstSet = diffuseDescriptorSet;
		writeDescriptorSet[0].dstBinding = DIFFUSE_COMBINED_IMAGE_SAMPLER_BINDING;
		writeDescriptorSet[0].dstArrayElement = 0;
		writeDescriptorSet[0].descriptorCount = 1;
		writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet[0].pImageInfo = &diffuseDescriptorImageInfo;
		writeDescriptorSet[0].pBufferInfo = nullptr;
		writeDescriptorSet[0].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device->handle, (uint32_t)writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);
	}
}

void GpuBreakout::Prepare(GameplayBreakout& _scene)
{
	// Get next image index
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

	size_t meshID = -1;
	vkBeginCommandBuffer(window->secondaryCommandBuffers[window->imageIndex], &commandBufferBeginInfo);
	{
		uint32_t instanceCounter = 0;
		uint32_t ballCount = 0;
		uint32_t paddleCount = 0;
		uint32_t blockCount = 0;

		// Parse scene
		{
			ListS<Ball>::Iterator ballIter = _scene.balls.GetHead();
			while (ballIter.node != nullptr)
			{
				instanceData[instanceCounter] =
				{
					ballIter.node->data.position.x,
					ballIter.node->data.position.y,
					ballIter.node->data.radius,
					ballIter.node->data.radius
				};
				++instanceCounter;
				ballIter.Next();
			}
			ballCount = instanceCounter;

			ListS<Paddle>::Iterator paddleIter = _scene.paddles.GetHead();
			while (paddleIter.node != nullptr)
			{
				instanceData[instanceCounter] =
				{
					paddleIter.node->data.position.x,
					paddleIter.node->data.position.y,
					paddleIter.node->data.size.x,
					paddleIter.node->data.size.y
				};
				++instanceCounter;
				paddleIter.Next();
			}
			paddleCount = instanceCounter - ballCount;

			ListS<Block>::Iterator blockIter = _scene.blocks.GetHead();
			while (blockIter.node != nullptr)
			{
				instanceData[instanceCounter] =
				{
					blockIter.node->data.position.x,
					blockIter.node->data.position.y,
					blockIter.node->data.size.x,
					blockIter.node->data.size.y
				};
				++instanceCounter;
				blockIter.Next();
			}
			blockCount = instanceCounter - (ballCount + paddleCount);
		}

		/// Sprite 2D
		// Pipeline
		vkCmdBindPipeline(window->secondaryCommandBuffers[window->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, sprite2DPipeline);
		{
			// Descriptor
			std::vector<VkDescriptorSet> descriptorSets = { diffuseDescriptorSet };
			vkCmdBindDescriptorSets(window->secondaryCommandBuffers[window->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseVert4PipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);
			float aspect = (float)window->extent.width / (float)window->extent.height;
			vkCmdPushConstants(window->secondaryCommandBuffers[window->imageIndex], diffuseVert4PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &aspect);

			/// Ball
			meshID = BALL_MESH_ID;
			// Bindings
			{
				std::vector<VkBuffer> vertexBuffers = { vertexBuffer.handle, modelMatricesBuffer.handle };
				std::vector<VkDeviceSize> vertexOffsets = { offsetsAndSizes[meshID].vertexOffset, 0 * INSTANCE_DATA_SIZE };
				vkCmdBindVertexBuffers(window->secondaryCommandBuffers[window->imageIndex], 0, (uint32_t)vertexBuffers.size(), vertexBuffers.data(), vertexOffsets.data());
				vkCmdBindIndexBuffer(window->secondaryCommandBuffers[window->imageIndex], indexBuffer.handle, offsetsAndSizes[meshID].indexOffset, VK_INDEX_TYPE_UINT16);

				vkCmdDrawIndexed(window->secondaryCommandBuffers[window->imageIndex], offsetsAndSizes[meshID].indexCount, ballCount, 0, 0, 0);
			}

			/// Paddle
			meshID = SQUARE_MESH_ID;
			// Bindings
			{
				std::vector<VkBuffer> vertexBuffers = { vertexBuffer.handle, modelMatricesBuffer.handle };
				std::vector<VkDeviceSize> vertexOffsets = { offsetsAndSizes[meshID].vertexOffset, ballCount * INSTANCE_DATA_SIZE };
				vkCmdBindVertexBuffers(window->secondaryCommandBuffers[window->imageIndex], 0, (uint32_t)vertexBuffers.size(), vertexBuffers.data(), vertexOffsets.data());
				vkCmdBindIndexBuffer(window->secondaryCommandBuffers[window->imageIndex], indexBuffer.handle, offsetsAndSizes[meshID].indexOffset, VK_INDEX_TYPE_UINT16);

				vkCmdDrawIndexed(window->secondaryCommandBuffers[window->imageIndex], offsetsAndSizes[meshID].indexCount, paddleCount, 0, 0, 0);
			}

			/// Blocks
			// Bindings
			{
				std::vector<VkBuffer> vertexBuffers = { vertexBuffer.handle, modelMatricesBuffer.handle };
				std::vector<VkDeviceSize> vertexOffsets = { offsetsAndSizes[meshID].vertexOffset, (ballCount + paddleCount) * INSTANCE_DATA_SIZE };
				vkCmdBindVertexBuffers(window->secondaryCommandBuffers[window->imageIndex], 0, (uint32_t)vertexBuffers.size(), vertexBuffers.data(), vertexOffsets.data());
				vkCmdBindIndexBuffer(window->secondaryCommandBuffers[window->imageIndex], indexBuffer.handle, offsetsAndSizes[meshID].indexOffset, VK_INDEX_TYPE_UINT16);

				vkCmdDrawIndexed(window->secondaryCommandBuffers[window->imageIndex], offsetsAndSizes[meshID].indexCount, blockCount, 0, 0, 0);
			}
		}
	}
	vkEndCommandBuffer(window->secondaryCommandBuffers[window->imageIndex]);
}
void GpuBreakout::Render()
{
	// Record primary buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	vkWaitForFences(device->handle, 1, &setupFence, VK_TRUE, -1);
	vkResetFences(device->handle, 1, &setupFence);

	vkBeginCommandBuffer(graphicsCommandBuffer, &commandBufferBeginInfo);

	// Fill Staging
	{
		VkU::FillBufferInfo fillBufferInfo;
		fillBufferInfo.vkDevice = device->handle;

		//fillBufferInfo.sourceBuffer = viewProjectionStagingBuffer;
		//fillBufferInfo.datas = { VkU::Data::GetData(viewProjection, sizeof(Math3D::Mat4) * 2) };
		//VkU::FillBuffer(viewProjectionBuffer, fillBufferInfo);

		/// TODO: change update method
		fillBufferInfo.sourceBuffer = modelMatricesStagingBuffer;
		fillBufferInfo.datas = { VkU::Data::GetData(instanceData.data(), sizeof(Math3D::Vec4) * instanceData.size()) };
		VkU::FillBuffer(modelMatricesBuffer, fillBufferInfo);
	}

	// Copy from staging to device buffers
	{
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;

		//bufferCopy.size = sizeof(Math3D::Mat4) * 2;
		//vkCmdCopyBuffer(graphicsCommandBuffer, viewProjectionStagingBuffer.handle, viewProjectionBuffer.handle, 1, &bufferCopy);

		bufferCopy.size = sizeof(Math3D::Mat4) * instanceData.size();
		vkCmdCopyBuffer(graphicsCommandBuffer, modelMatricesStagingBuffer.handle, modelMatricesBuffer.handle, 1, &bufferCopy);
	}

	// Render
	{
		VkClearValue clearColor[2];
		clearColor[0].color = { 0.05f, 0.2f, 0.2f, 0.0f };
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
	}
}

void GpuBreakout::ShutDown()
{
	std::vector<VkU::Window*> windows = { window };
	vkWaitForFences(device->handle, 1, &setupFence, VK_TRUE, ~0U);
	for (size_t iWindow = 0; iWindow != windows.size(); ++iWindow)
		vkWaitForFences(device->handle, (uint32_t)windows[iWindow]->fences.size(), windows[iWindow]->fences.data(), VK_TRUE, ~0U);

	//vkDeviceWaitIdle(device->handle);

	/// Setup
	// Descriptor Pool
	vkDestroyDescriptorPool(device->handle, descriptorPool, nullptr);

	/// Load
	// Buffers
	vkDestroyBuffer(device->handle, indexBuffer.handle, nullptr);
	vkFreeMemory(device->handle, indexBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, vertexBuffer.handle, nullptr);
	vkFreeMemory(device->handle, vertexBuffer.memory, nullptr);

	vkDestroyBuffer(device->handle, modelMatricesStagingBuffer.handle, nullptr);
	vkFreeMemory(device->handle, modelMatricesStagingBuffer.memory, nullptr);
	vkDestroyBuffer(device->handle, modelMatricesBuffer.handle, nullptr);
	vkFreeMemory(device->handle, modelMatricesBuffer.memory, nullptr);

	//vkDestroyBuffer(device->handle, viewProjectionStagingBuffer.handle, nullptr);
	//vkFreeMemory(device->handle, viewProjectionStagingBuffer.memory, nullptr);
	//vkDestroyBuffer(device->handle, viewProjectionBuffer.handle, nullptr);
	//vkFreeMemory(device->handle, viewProjectionBuffer.memory, nullptr);

	// Textures
	vkDestroyImageView(device->handle, diffuseTexture.view, nullptr);
	vkDestroyImage(device->handle, diffuseTexture.handle, nullptr);
	vkFreeMemory(device->handle, diffuseTexture.memory, nullptr);

	//vkDestroyImageView(device->handle, fontTexture.view, nullptr);
	//vkDestroyImage(device->handle, fontTexture.handle, nullptr);
	//vkFreeMemory(device->handle, fontTexture.memory, nullptr);
	//
	//vkDestroyImageView(device->handle, rocksDiffuseTexture.view, nullptr);
	//vkDestroyImage(device->handle, rocksDiffuseTexture.handle, nullptr);
	//vkFreeMemory(device->handle, rocksDiffuseTexture.memory, nullptr);
	//
	//vkDestroyImageView(device->handle, rocksNormalTexture.view, nullptr);
	//vkDestroyImage(device->handle, rocksNormalTexture.handle, nullptr);
	//vkFreeMemory(device->handle, rocksNormalTexture.memory, nullptr);

	// Pipeline
	for (size_t iPipeline = 0; iPipeline != graphicsPipelines.size(); ++iPipeline)
		vkDestroyPipeline(device->handle, graphicsPipelines[iPipeline], nullptr);

	// Shader Module

	for(size_t iShader = 0; iShader != shaderModules.size(); ++iShader)
		vkDestroyShaderModule(device->handle, shaderModules[iShader], nullptr);

	//vkDestroyShaderModule(device->handle, vertexTextShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, geometryTextShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, fragmentTextShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, vertexParallaxOcclusionShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, fragmentParallaxOcclusionShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, vertex2DSpriteShaderModule, nullptr);
	//vkDestroyShaderModule(device->handle, fragment2DSpriteShaderModule, nullptr);

	/// Init
	// Sampler
	vkDestroySampler(device->handle, nearestSampler, nullptr);
	vkDestroySampler(device->handle, linearSampler, nullptr);

	// PipelineLayout
	vkDestroyPipelineLayout(device->handle, diffuseVert4PipelineLayout, nullptr);

	//vkDestroyPipelineLayout(device->handle, fontPush16Vert16Frag16GeomPipelineLayout, nullptr);
	//vkDestroyPipelineLayout(device->handle, mvpDiffuseNormalPush8Vert8FragPipelineLayout, nullptr);

	// DescriptorSetLayout
	vkDestroyDescriptorSetLayout(device->handle, diffuseDescriptorSetLayout, nullptr);

	//vkDestroyDescriptorSetLayout(device->handle, fontDescriptorSetLayout, nullptr);
	//vkDestroyDescriptorSetLayout(device->handle, diffuseNormalDescriptorSetLayout, nullptr);
	//vkDestroyDescriptorSetLayout(device->handle, mvpDescriptorSetLayout, nullptr);

	// Window Resources
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
			windows[iWindow]->depthImage = nullptr;
		}

		for (size_t iView = 0; iView != windows[iWindow]->views.size(); ++iView)
			vkDestroyImageView(device->handle, windows[iWindow]->views[iView], nullptr);

		vkDestroySwapchainKHR(device->handle, windows[iWindow]->swapchain, nullptr);
	}

	// CommandPool
	vkDestroyCommandPool(device->handle, graphicsCommandPool, nullptr);

	// RenderPass
	vkDestroyRenderPass(device->handle, renderPass, nullptr);

	// Fence
	vkDestroyFence(device->handle, setupFence, nullptr);
}

RECT GpuBreakout::GetClientScreenSpace()
{
	RECT rect;
	GetClientRect(window->hWnd, &rect); // get client coords
	ClientToScreen(window->hWnd, reinterpret_cast<POINT*>(&rect.left)); // convert top-left
	ClientToScreen(window->hWnd, reinterpret_cast<POINT*>(&rect.right)); // convert bottom-right

	return rect;
}
