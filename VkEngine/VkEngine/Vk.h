#ifndef	VK_H
#define VK_H

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>
#define VK_RESERVED_FOR_FUTURE_USE 0
#define VK_FLAGS_NONE 0

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if _DEBUG
#include <iostream>
#endif

#include <vector>

namespace VkS
{
	enum VKU_RESULT
	{
		VKU_SUCCESS,
		VKU_INCOMPLETE,
		VKU_TIMEOUT,
		VKU_UNKNOWN,

		VKU_LARGEST_SUCCESS_VALUE,

		VKU_NO_HOST_MEMORY,
		VKU_NO_DEVICE_MEMORY,
		VKU_BAD_IMPLEMENTATION,
		VKU_MISSING_LAYER,
		VKU_MISSING_EXTENSION,
		VKU_INCOMPATIBLE_DRIVER,
		VKU_WINDOW_WIN32_REGISTRATION,
		VKU_WINDOW_WIN32_HWND_IS_NULL,
		VKU_NO_COMPATIBLE_PHYSICAL_DEVICE,
		VKU_MISSING_FEATURE,
		VKU_TOO_MANY_OBJECTS,
		VKU_DEVICE_LOST,
		VKU_SURFACE_LOST,
		VKU_NATIVE_WINDOW_IN_USE,
		VKU_INVALID_EXTERNAL_HANDLE,
		VKU_WRONG_RENDERPASS_COUNT,
		VKU_SHADER_FILE_NOT_OPENED,
		VKU_MEMORY_MAP_FAILED,
		VKU_FRAGMENTED_POOL,

		VKU_IMAGE_FILE_NOT_OPENED,
		VKU_IMAGE_FILE_READING_FAILURE,
		VKU_IMAGE_FILE_NOT_UNCOMPRESSED_TGA,
		VKU_IMAGE_FILE_UNEXPECTED_FORMAT,
	};

	// GpuController
	struct Texture
	{
		VkImage handle;
		VkDeviceMemory memory;
		VkImageView view;
		VkExtent2D extent;
	};
	struct Buffer
	{
		VkBuffer handle;
		VkDeviceMemory memory;
		VkDeviceSize size;
	};
	struct PipelineData
	{
		struct ShaderStageCreateInfo
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
		};
		std::vector<ShaderStageCreateInfo>						shaderStageCreateInfos;

		struct VertexInputBindingDescriptions
		{
			std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
		};
		std::vector<VertexInputBindingDescriptions>				vertexInputBindingDescriptions;
		struct VertexInputAttributeDescriptions
		{
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		};
		std::vector<VertexInputAttributeDescriptions>			vertexInputAttributeDescriptions;
		std::vector<VkPipelineVertexInputStateCreateInfo>		pipelineVertexInputStateCreateInfos;

		std::vector<VkPipelineInputAssemblyStateCreateInfo>		pipelineInputAssemblyStateCreateInfos;

		std::vector<VkPipelineTessellationStateCreateInfo>		pipelineTessellationStateCreateInfo;

		struct Viewports
		{
			std::vector<VkViewport> viewports;
		};
		std::vector<Viewports>									viewports;
		struct Scissors
		{
			std::vector<VkRect2D> scissors;
		};
		std::vector<Scissors>									scissors;
		std::vector<VkPipelineViewportStateCreateInfo>			pipelineViewportStateCreateInfos;

		std::vector<VkPipelineRasterizationStateCreateInfo>		pipelineRasterizationStateCreateInfos;

		std::vector<VkPipelineMultisampleStateCreateInfo>		pipelineMultisampleStateCreateInfos;

		std::vector<VkPipelineDepthStencilStateCreateInfo>		pipelineDepthStencilStateCreateInfos;

		struct PipelineColorBlendAttachmentState
		{
			std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
		};
		std::vector<PipelineColorBlendAttachmentState>			pipelineColorBlendAttachmentStates;
		std::vector<VkPipelineColorBlendStateCreateInfo>		pipelineColorBlendStateCreateInfos;
	};
	struct ImageData
	{
		VkExtent2D extent2D;
		VkDeviceSize size;
		VkFormat format;
		void* data;
	};

	// Gpu
	struct Instance
	{
		VkInstance handle;
#if _DEBUG
		VkDebugReportCallbackEXT debugReportCallback;
#endif

		struct PhysicalDevice
		{
			VkPhysicalDevice handle;

			VkPhysicalDeviceFeatures				features;
			VkPhysicalDeviceMemoryProperties		memoryProperties;
			VkPhysicalDeviceProperties				properties;
			std::vector<VkQueueFamilyProperties>	queueFamilyProperties;
			std::vector<VkBool32>					queueFamilyPresentable;
			VkFormat depthFormat;
		};
		std::vector<PhysicalDevice> physicalDevices;
	};
	struct Window
	{
		HINSTANCE hInstance;
		HWND hWnd;
		const char* name;

		VkSurfaceKHR surface;
		VkSurfaceFormatKHR colorFormat;
		VkCompositeAlphaFlagBitsKHR compositeAlpha;
		VkPresentModeKHR presentMode;

		VkSwapchainKHR swapchain;
		VkExtent2D extent;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		std::vector<VkFramebuffer> framebuffers;

		std::vector<VkFence> fences;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderDoneSemaphore;
		std::vector<VkCommandBuffer> commandBuffers;

		Texture* depthImage;
		uint32_t imageIndex;
	};
	struct Device
	{
		VkDevice handle;
		Instance::PhysicalDevice* physicalDevice;
		std::vector<Window*> windows;

		struct QueueGroup
		{
			uint32_t					familyIndex;
			uint32_t					firstIndex;
			VkQueueFlags				flags;
			bool						presentability;
			float						priority;
			std::vector<VkQueue>		queues;

			struct Requirements
			{
				VkQueueFlags	flags;
				bool			presentability;
				float			priority;
				uint32_t		count;

				static inline Requirements GetRequirements(VkQueueFlags	_flags, bool _presentability, float _priority, uint32_t _count)
				{
					return { _flags, _presentability, _priority, _count };
				}
			};
		};
		std::vector<QueueGroup> queueGroups;
	};
}

namespace VkU
{
	static VkResult vkApiResult = VK_SUCCESS;

	// Gpu
	struct CreateInstanceInfo
	{
		const char* applicationName;
		uint32_t applicationVersion;
		const char* engineName;
		uint32_t engineVersion;
		std::vector<const char*> instanceLayerNames;
		std::vector<const char*> instanceExtensionNames;
#if _DEBUG
		VkDebugReportFlagsEXT debugReportFlags;
		PFN_vkDebugReportCallbackEXT debugReportCallback;
#endif
		std::vector<VkFormat> preferedDepthFormats;
	};
	struct CreateWindowInfo
	{
		int x;
		int y;
		int width;
		int height;
		const char* title;
		const char* name;
		WNDPROC wndProc;

		VkInstance instance;
		bool hasDepthBuffer;
	};
	struct CreateDeviceInfo
	{
		VkS::Instance* instance;
		VkPhysicalDeviceFeatures requestedFeatures;
		struct QueueProperties
		{
			VkS::Device::QueueGroup::Requirements requirements;

			static inline QueueProperties GetQueueProperties(VkS::Device::QueueGroup::Requirements _requirements)
			{
				return { _requirements };
			}
		};
		std::vector<QueueProperties> queuesProperties;
		std::vector<VkS::Window*> windows;
		struct WindowProperties
		{
			std::vector<VkCompositeAlphaFlagBitsKHR> preferedCompositeAlphas;
			std::vector<VkPresentModeKHR> preferedPresentModes;

			static inline WindowProperties GetWindowProperties(std::vector<VkCompositeAlphaFlagBitsKHR> _preferedCompositeAlphas, std::vector<VkPresentModeKHR> _preferedPresentModes)
			{
				return { _preferedCompositeAlphas, _preferedPresentModes };
			}
		};
		std::vector<WindowProperties> windowsProperties;
		std::vector<const char*> deviceLayerNames;
		std::vector<const char*> deviceExtensionNames;
	};

	// GpuController
	struct CreateWindowResourcesInfo
	{
		VkDevice vkDevice;

		VkS::Instance::PhysicalDevice* physicalDevice;
		VkFence setupFence;
		VkCommandBuffer setupCommandBuffer;
		VkQueue setupQueue;
		VkCommandPool graphicsCommandPool;

		uint32_t imageCount;

		VkRenderPass renderPass;
	};
	struct CreateShaderModuleInfo
	{
		VkDevice vkDevice;
		const char* filename;
	};

	struct CreateBufferInfo
	{
		VkDevice vkDevice;
		VkS::Instance::PhysicalDevice* physicalDevice;
		VkDeviceSize size;
		VkBufferUsageFlags bufferUsageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;
	};
	struct FillBufferInfo
	{
		VkDevice vkDevice;
		VkS::Buffer dstBuffer;
		VkDeviceSize offset;
		VkDeviceSize size;
		void* data;
	};
	struct CopyBufferInfo
	{
		VkDevice vkDevice;
		VkFence setupFence;
		VkQueue setupQueue;
		VkCommandBuffer setupCommandBuffer;
		VkBuffer srcBuffer;
		std::vector<VkBufferCopy> copyRegions;
		VkBuffer dstBuffer;
	};

	struct LoadImageDataInfo
	{
		const char* fileName;
	};
	struct CreateTextureInfo
	{
		VkDevice vkDevice;
		VkS::Instance::PhysicalDevice* physicalDevice;
		VkFormat format;
		VkExtent3D extent3D;
		VkImageUsageFlags usage;
		VkImageTiling tiling;
		VkMemoryPropertyFlags memoryPropertyFlags;
		VkImageAspectFlags aspect;
	};
	struct FillTextureInfo
	{
		VkDevice vkDevice;
		VkS::Texture dstTexture;
		uint32_t width;
		uint32_t height;
		VkDeviceSize size;
		void* data;
	};
	struct CopyTextureInfo
	{
		VkDevice vkDevice;
		VkFence setupFence;
		VkCommandBuffer setupCommandBuffer;
		VkQueue setupQueue;
		VkS::Texture srcTexture;
		VkS::Texture dstTexture;
	};
}

namespace VkA
{
	// Util
	uint32_t FindMemoryTypeIndex(VkS::Instance::PhysicalDevice* _physicalDevice, VkMemoryRequirements _memoryRequirements, VkMemoryPropertyFlags _memoryPropertyFlags);

	// Gpu
#if _DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT _flags, VkDebugReportObjectTypeEXT _objType, uint64_t _obj, size_t _location, int32_t _code, const char* _layerPrefix, const char* _msg, void* _userData)
	{
		enum COLORS
		{
			BLACK = 0,
			BLUE = 1,
			GREEN = 2,
			CYAN = 3,
			RED = 4,
			PINK = 5,
			YELLOW = 6,
			WHITE = 7,
		};

		enum INTENSITY
		{
			LOW = 0,
			HIGH = 8
		};

		if (_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WHITE + HIGH) + ((BLACK + LOW) * 16));
			std::cout << "	INFORMATION:";
		}
		else if (_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (YELLOW + HIGH) + ((BLACK + LOW) * 16));
			std::cout << "WARNING:";
		}
		else if (_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (PINK + HIGH) + ((BLACK + LOW) * 16));
			std::cout << "PERFORMANCE:";
		}
		else if (_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (RED + HIGH) + ((BLACK + LOW) * 16));
			std::cout << "ERROR:";
		}
		else if (_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (GREEN + HIGH) + ((BLACK + LOW) * 16));
			std::cout << "	DEBUG:";
		}

		std::cout << _msg << '\n';
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WHITE + LOW) + ((BLACK + LOW) * 16));

		return VK_FALSE; // VK_FALSE wont't abort the function that made this call
	}
#endif
	static LRESULT WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
	{
		switch (_uMsg)
		{
		case WM_CLOSE:
			//Engine::endApp = true;
			break;
		case WM_PAINT:
			ValidateRect(_hWnd, NULL);
			break;
		case WM_SIZE:
			//if (_wParam == SIZE_MINIMIZED)
			;//Renderer::currentRenderer->RecreateSwapchain(LOWORD(_lParam), HIWORD(_lParam));
			 //else if (_wParam == SIZE_MAXIMIZED)
			;//Renderer::currentRenderer->RecreateSwapchain(LOWORD(_lParam), HIWORD(_lParam));
			 //else if (_wParam == SIZE_RESTORED)
			;//Renderer::currentRenderer->RecreateSwapchain(0, 0);
			break;
		default:
			break;
		}

		return (DefWindowProc(_hWnd, _uMsg, _wParam, _lParam));
	}

	VkS::VKU_RESULT CreateInstance(VkS::Instance& _instance, VkU::CreateInstanceInfo _createInstanceInfo);
	VkS::VKU_RESULT CreateOSWindow(VkS::Window& _window, VkU::CreateWindowInfo _createWindowInfo);
	VkS::VKU_RESULT CreateDevice(VkS::Device& _device, VkU::CreateDeviceInfo _createDeviceInfo);

	// GpuController
	VkS::VKU_RESULT CreateWindowResources(VkS::Window& _window, VkU::CreateWindowResourcesInfo _createWindowResourcesInfo);
	VkS::VKU_RESULT CreateShaderModule(VkShaderModule& _shaderModule, VkU::CreateShaderModuleInfo _createShaderModuleInfo);

	VkS::VKU_RESULT CreateBuffer(VkS::Buffer& _buffer, VkU::CreateBufferInfo _createBufferInfo);
	VkS::VKU_RESULT FillBuffer(VkU::FillBufferInfo _fillBufferInfo);
	VkS::VKU_RESULT CopyBuffer(VkU::CopyBufferInfo _copyBufferInfo);

	VkS::VKU_RESULT LoadImageData(VkS::ImageData& _imageData, VkU::LoadImageDataInfo _loadImageDataInfo);
	VkS::VKU_RESULT CreateTexture(VkS::Texture& _texture, VkU::CreateTextureInfo _createTextureInfo);
	VkS::VKU_RESULT FillTexture(VkU::FillTextureInfo _fillTextureInfo);
	VkS::VKU_RESULT CopyTexture(VkU::CopyTextureInfo _copyTextureInfo);
}

#endif