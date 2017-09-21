#include "Gpu.h"

void Gpu::Init()
{
	// Instance
	{
		VkU::CreateInstanceInfo createInstanceInfo;
		createInstanceInfo.applicationName = "appName";
		createInstanceInfo.applicationVersion = 0;
		createInstanceInfo.engineName = "engineName";
		createInstanceInfo.engineVersion = 0;
		createInstanceInfo.instanceLayerNames = { //"VK_LAYER_LUNARG_swapchain",
#if _DEBUG
			"VK_LAYER_LUNARG_standard_validation",
#endif
		};
		createInstanceInfo.instanceExtensionNames = { "VK_KHR_surface", "VK_KHR_win32_surface",
#if _DEBUG
			"VK_EXT_debug_report",
#endif
		};
#if _DEBUG
		createInstanceInfo.debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		createInstanceInfo.debugReportCallback = VkA::DebugReportCallback;
#endif
		createInstanceInfo.preferedDepthFormats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D16_UNORM, VK_FORMAT_D16_UNORM_S8_UINT };
		VkA::CreateInstance(instance, createInstanceInfo);
	}

	// Window
	{
		windows.resize(1);
		VkU::CreateWindowInfo createWindowInfo;
		createWindowInfo.x = 0;
		createWindowInfo.y = ~0U;
		createWindowInfo.width = 800;
		createWindowInfo.height = 600;
		createWindowInfo.title = "title";
		createWindowInfo.name = "name";
		createWindowInfo.wndProc = VkA::WndProc;
		createWindowInfo.instance = instance.handle;
		createWindowInfo.hasDepthBuffer = true;
		VkA::CreateOSWindow(windows[0], createWindowInfo);
	}

	// Device
	{
		VkPhysicalDeviceFeatures features = {};
		features.fillModeNonSolid = VK_TRUE;
		features.multiViewport = VK_TRUE;
		features.tessellationShader = VK_TRUE;

		VkU::CreateDeviceInfo createDeviceInfo;
		createDeviceInfo.instance = &instance;
		createDeviceInfo.requestedFeatures = features;
		createDeviceInfo.windows = { &windows[0] };
		createDeviceInfo.windowsProperties = {
			VkU::CreateDeviceInfo::WindowProperties::GetWindowProperties({ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, },{ VK_PRESENT_MODE_MAILBOX_KHR, }),
			VkU::CreateDeviceInfo::WindowProperties::GetWindowProperties({ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, },{ VK_PRESENT_MODE_MAILBOX_KHR, }),
		};
		createDeviceInfo.queuesProperties = {
			VkU::CreateDeviceInfo::QueueProperties::GetQueueProperties(VkS::Device::QueueGroup::Requirements::GetRequirements(VK_QUEUE_GRAPHICS_BIT, true, 1.0f, 1)),
			//VkU::CreateDeviceInfo::QueueProperties::GetQueueProperties(VkS::Device::QueueGroup::Requirements::GetRequirements(VK_QUEUE_COMPUTE_BIT, false, 1.0f, 1)),
		};
		createDeviceInfo.deviceLayerNames = {
#if _DEBUG
			"VK_LAYER_LUNARG_standard_validation",
#endif
		};
		createDeviceInfo.deviceExtensionNames = { "VK_KHR_swapchain", };
		VkA::CreateDevice(device, createDeviceInfo);
	}
}

void Gpu::ShutDown()
{
	// Window
	for (size_t iWindow = 0; iWindow != windows.size(); ++iWindow)
	{
		vkDestroySurfaceKHR(instance.handle, windows[iWindow].surface, nullptr);

		DestroyWindow(windows[iWindow].hWnd);
		UnregisterClass(windows[iWindow].name, GetModuleHandle(NULL));
	}

	// Device
	vkDestroyDevice(device.handle, nullptr);
	device.physicalDevice = nullptr;
	device.queueGroups.clear();

	// Instance
	instance.physicalDevices.clear();
#if _DEBUG
	PFN_vkDestroyDebugReportCallbackEXT FP_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance.handle, "vkDestroyDebugReportCallbackEXT");
	FP_vkDestroyDebugReportCallbackEXT(instance.handle, instance.debugReportCallback, nullptr);
#endif
	vkDestroyInstance(instance.handle, nullptr);
}
