#include "Vk.h"

#define PHYSICAL_DEVICE_CHOICE_INDEX 0

#include<fstream>

uint32_t VkA::FindMemoryTypeIndex(VkS::Instance::PhysicalDevice* _physicalDevice, VkMemoryRequirements _memoryRequirements, VkMemoryPropertyFlags _memoryPropertyFlags)
{
	for (uint32_t i = 0; i != _physicalDevice->memoryProperties.memoryTypeCount; ++i)
	{
		if ((_memoryRequirements.memoryTypeBits & (1 << i)) && (_physicalDevice->memoryProperties.memoryTypes[i].propertyFlags & _memoryPropertyFlags) == _memoryPropertyFlags)
		{
			return i;
		}
	}

	return -1;
}

VkS::VKU_RESULT VkA::CreateInstance(VkS::Instance & _instance, VkU::CreateInstanceInfo _createInstanceInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	// Instance
	{
		VkApplicationInfo applicationInfo;
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = _createInstanceInfo.applicationName;
		applicationInfo.applicationVersion = _createInstanceInfo.applicationVersion;
		applicationInfo.pEngineName = _createInstanceInfo.engineName;
		applicationInfo.engineVersion = _createInstanceInfo.engineVersion;
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledLayerCount = (uint32_t)_createInstanceInfo.instanceLayerNames.size();
		instanceCreateInfo.ppEnabledLayerNames = _createInstanceInfo.instanceLayerNames.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)_createInstanceInfo.instanceExtensionNames.size();
		instanceCreateInfo.ppEnabledExtensionNames = _createInstanceInfo.instanceExtensionNames.data();

		VkU::vkApiResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance.handle);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_INITIALIZATION_FAILED:	return VkS::VKU_BAD_IMPLEMENTATION;
		case VK_ERROR_LAYER_NOT_PRESENT:		return VkS::VKU_MISSING_LAYER;
		case VK_ERROR_EXTENSION_NOT_PRESENT:	return VkS::VKU_MISSING_EXTENSION;
		case VK_ERROR_INCOMPATIBLE_DRIVER:		return VkS::VKU_INCOMPATIBLE_DRIVER;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	// Debug
#if _DEBUG
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pNext = nullptr;
	debugReportCallbackCreateInfo.flags = _createInstanceInfo.debugReportFlags;
	debugReportCallbackCreateInfo.pfnCallback = _createInstanceInfo.debugReportCallback;
	debugReportCallbackCreateInfo.pUserData = nullptr;

	PFN_vkCreateDebugReportCallbackEXT FP_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance.handle, "vkCreateDebugReportCallbackEXT");
	VkU::vkApiResult = FP_vkCreateDebugReportCallbackEXT(_instance.handle, &debugReportCallbackCreateInfo, nullptr, &_instance.debugReportCallback);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}
#endif

	// Physical Devices
	{
		uint32_t propertyCount = 0;
		VkU::vkApiResult = vkEnumeratePhysicalDevices(_instance.handle, &propertyCount, nullptr);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_INCOMPLETE:						vkuResult = VkS::VKU_INCOMPLETE; break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_INITIALIZATION_FAILED:	return VkS::VKU_BAD_IMPLEMENTATION;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
		std::vector<VkPhysicalDevice> physicalDevicesHandles(propertyCount);
		VkU::vkApiResult = vkEnumeratePhysicalDevices(_instance.handle, &propertyCount, physicalDevicesHandles.data());
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_INCOMPLETE:						vkuResult = VkS::VKU_INCOMPLETE; break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_INITIALIZATION_FAILED:	return VkS::VKU_BAD_IMPLEMENTATION;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}

		_instance.physicalDevices.resize(physicalDevicesHandles.size());
		for (size_t i = 0; i != physicalDevicesHandles.size(); ++i)
		{
			// Get properties
			_instance.physicalDevices[i].handle = physicalDevicesHandles[i];

			vkGetPhysicalDeviceProperties(_instance.physicalDevices[i].handle, &_instance.physicalDevices[i].properties);
			vkGetPhysicalDeviceFeatures(_instance.physicalDevices[i].handle, &_instance.physicalDevices[i].features);
			vkGetPhysicalDeviceMemoryProperties(_instance.physicalDevices[i].handle, &_instance.physicalDevices[i].memoryProperties);

			uint32_t propertyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(_instance.physicalDevices[i].handle, &propertyCount, nullptr);
			_instance.physicalDevices[i].queueFamilyProperties.resize(propertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(_instance.physicalDevices[i].handle, &propertyCount, _instance.physicalDevices[i].queueFamilyProperties.data());

			_instance.physicalDevices[i].queueFamilyPresentable.resize(_instance.physicalDevices[i].queueFamilyProperties.size());
			for (uint32_t j = 0; j != _instance.physicalDevices[i].queueFamilyPresentable.size(); ++j)
				_instance.physicalDevices[i].queueFamilyPresentable[j] = vkGetPhysicalDeviceWin32PresentationSupportKHR(_instance.physicalDevices[i].handle, j);

			// Get Depth
			_instance.physicalDevices[i].depthFormat = VK_FORMAT_UNDEFINED;

			for (uint32_t j = 0; j != _createInstanceInfo.preferedDepthFormats.size(); ++j)
			{
				VkFormatProperties formatProperties;
				vkGetPhysicalDeviceFormatProperties(_instance.physicalDevices[i].handle, _createInstanceInfo.preferedDepthFormats[j], &formatProperties);

				if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					_instance.physicalDevices[i].depthFormat = _createInstanceInfo.preferedDepthFormats[j];
					break;
				}
			}
		}
	}

	return vkuResult;
}
VkS::VKU_RESULT VkA::CreateOSWindow(VkS::Window & _window, VkU::CreateWindowInfo _createWindowInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	_window.hInstance = GetModuleHandle(NULL);
	_window.name = _createWindowInfo.name;

	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = _createWindowInfo.wndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = GetModuleHandle(NULL);
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = _window.name;
	wndClassEx.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClassEx))
		return VkS::VKU_WINDOW_WIN32_REGISTRATION;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = (long)_createWindowInfo.width;
	windowRect.bottom = (long)_createWindowInfo.height;
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	_window.hWnd = CreateWindowEx(
		0,
		_window.name,
		_createWindowInfo.title,
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		_createWindowInfo.x,
		_createWindowInfo.y,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	if (_window.hWnd == NULL)
		return VkS::VKU_WINDOW_WIN32_HWND_IS_NULL;

	uint32_t x = _createWindowInfo.x;
	uint32_t y = _createWindowInfo.y;
	if (_createWindowInfo.x == ~0U)
		x = (uint32_t)((screenWidth - windowRect.right) * 0.5f);

	if (_createWindowInfo.y == ~0U)
		y = (uint32_t)((screenHeight - windowRect.bottom) * 0.5f);

	if (_createWindowInfo.x == ~0U || _createWindowInfo.y == ~0U)
		SetWindowPos(_window.hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	ShowWindow(_window.hWnd, SW_SHOW);
	SetForegroundWindow(_window.hWnd);
	SetFocus(_window.hWnd);

	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo;
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.pNext = nullptr;
	win32SurfaceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	win32SurfaceCreateInfo.hinstance = _window.hInstance;
	win32SurfaceCreateInfo.hwnd = _window.hWnd;

	VkU::vkApiResult = vkCreateWin32SurfaceKHR(_createWindowInfo.instance, &win32SurfaceCreateInfo, nullptr, &_window.surface);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	if (_createWindowInfo.hasDepthBuffer == true)
	{
		_window.depthImage = new VkS::Texture;
		_window.depthImage->handle = VK_NULL_HANDLE;
		_window.depthImage->memory = VK_NULL_HANDLE;
		_window.depthImage->view = VK_NULL_HANDLE;
	}

	return vkuResult;
}
VkS::VKU_RESULT VkA::CreateDevice(VkS::Device & _device, VkU::CreateDeviceInfo _createDeviceInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	_device.windows = _createDeviceInfo.windows;

	/// Physical Devices & QueueGroups
	// List compatible physical devices
	std::vector<uint32_t> compatiblePhysicalDevices;

	struct LocalFunc2
	{
	public:
		struct QueueArrangement
		{
			struct Queue
			{
				std::vector<uint32_t>	compatibleFamilies;
				VkQueueFlags			flags;
				bool					presentability;
				float					priority;
				uint32_t				count;

				uint32_t				familyIndex;
				uint32_t				firstIndex;
			};
			std::vector<Queue> queues;
		};
	private:
		static bool PickQueuesIndicesRecursivelly(QueueArrangement& _queueArrangement, VkS::Instance::PhysicalDevice _physicalDevice, size_t _depth = 0, std::vector<uint32_t> _queueFamilyUseCount = {})
		{
			if (_depth == _queueArrangement.queues.size())
				return true; // Found valid pattern

			if (_queueFamilyUseCount.size() == 0)
				_queueFamilyUseCount.resize(_physicalDevice.queueFamilyProperties.size());

			std::vector<uint32_t> usableIndices = _queueArrangement.queues[_depth].compatibleFamilies;
			uint32_t queueCount = _queueArrangement.queues[_depth].count;

			for (size_t iUsableIndices = 0; iUsableIndices != usableIndices.size(); ++iUsableIndices)
			{
				uint32_t familyIndex = usableIndices[iUsableIndices];
				// Check space on queue
				if (_queueFamilyUseCount[familyIndex] + queueCount <= _physicalDevice.queueFamilyProperties[familyIndex].queueCount)
				{
					_queueFamilyUseCount[familyIndex] += queueCount;
					if (PickQueuesIndicesRecursivelly(_queueArrangement, _physicalDevice, _depth + 1, _queueFamilyUseCount))
					{
						_queueArrangement.queues[_depth].familyIndex = familyIndex;
						_queueArrangement.queues[_depth].firstIndex = _queueFamilyUseCount[familyIndex] - queueCount;
						return true;
					}
					else
						_queueFamilyUseCount[familyIndex] -= queueCount;
				}
			}

			return false;
		}
	public:
		static QueueArrangement GetQueueArrangement(std::vector<VkS::Device::QueueGroup::Requirements> _requirements, VkS::Instance::PhysicalDevice _physicalDevice, std::vector<VkS::Window*> _windows)
		{
			QueueArrangement queueArrangement;

			queueArrangement.queues.resize(_requirements.size());
			for (size_t iRequirements = 0; iRequirements != _requirements.size(); ++iRequirements)
			{
				queueArrangement.queues[iRequirements].count = _requirements[iRequirements].count;
				queueArrangement.queues[iRequirements].flags = _requirements[iRequirements].flags;
				queueArrangement.queues[iRequirements].presentability = _requirements[iRequirements].presentability;
				queueArrangement.queues[iRequirements].priority = _requirements[iRequirements].priority;

				queueArrangement.queues[iRequirements].familyIndex = -1;
				queueArrangement.queues[iRequirements].firstIndex = -1;
			}

			// Get compatible family indices
			for (size_t iRequirements = 0; iRequirements != _requirements.size(); ++iRequirements)
			{
				// List valid indices
				for (size_t iQueueFamily = 0; iQueueFamily != _physicalDevice.queueFamilyProperties.size(); ++iQueueFamily)
				{
					// Check surfaces
					VkBool32 surfaceSupported = VK_TRUE;
					if (_requirements[iRequirements].presentability == true)
					{
						for (size_t iWindow = 0; iWindow != _windows.size(); ++iWindow)
						{
							vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice.handle, (uint32_t)iQueueFamily, (*_windows[iWindow]).surface, &surfaceSupported);

							if (surfaceSupported == VK_FALSE)
								break;
						}
					}

					if (surfaceSupported == VK_FALSE)
						continue;

					// Check properties
					if ((_requirements[iRequirements].flags & _physicalDevice.queueFamilyProperties[iQueueFamily].queueFlags) != _requirements[iRequirements].flags)
						continue;

					if (_requirements[iRequirements].presentability == true && _physicalDevice.queueFamilyPresentable[iQueueFamily] == VK_FALSE)
						continue;

					if (_requirements[iRequirements].count > _physicalDevice.queueFamilyProperties[iQueueFamily].queueCount)
						continue;

					queueArrangement.queues[iRequirements].compatibleFamilies.push_back((uint32_t)iQueueFamily);
				}
			}

			// Pick family index
			bool compatible = PickQueuesIndicesRecursivelly(queueArrangement, _physicalDevice);

			if (compatible == true)
				return queueArrangement;
			else
				return {};

			return queueArrangement;
		}
	};

	// List of queues
	std::vector<LocalFunc2::QueueArrangement> physicalDevicesQueueGroups(_createDeviceInfo.instance->physicalDevices.size());
	for (size_t iPhysicalDevice = 0; iPhysicalDevice != _createDeviceInfo.instance->physicalDevices.size(); ++iPhysicalDevice)
	{
		// Check requested features
		bool featureCompatible = true;

		VkBool32* requiredFeatures = (VkBool32*)&_createDeviceInfo.requestedFeatures;
		VkBool32* availableFeatures = (VkBool32*)&_createDeviceInfo.instance->physicalDevices[iPhysicalDevice].features;
		for (size_t j = 0; j != sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++j)
		{
			if (requiredFeatures[j] == VK_TRUE && availableFeatures == VK_FALSE)
			{
				featureCompatible = false;
				break;
			}
		}

		if (featureCompatible == false)
			continue;

		// Check / Get requested queues
		std::vector<VkS::Device::QueueGroup::Requirements> queuesRequirements(_createDeviceInfo.queuesProperties.size());
		for (size_t iRequirement = 0; iRequirement != _createDeviceInfo.queuesProperties.size(); ++iRequirement)
			queuesRequirements[iRequirement] = _createDeviceInfo.queuesProperties[iRequirement].requirements;

		_device.windows;
		LocalFunc2::QueueArrangement queueArrangement = LocalFunc2::GetQueueArrangement(queuesRequirements, _createDeviceInfo.instance->physicalDevices[iPhysicalDevice], _device.windows);

		if (queueArrangement.queues.size() > 0)
		{
			compatiblePhysicalDevices.push_back((uint32_t)iPhysicalDevice);
			physicalDevicesQueueGroups[iPhysicalDevice] = queueArrangement;
		}
	}

	// Pick a physical device if one is available
	if (compatiblePhysicalDevices.size() > 0)
	{
		uint32_t physicalDeviceIndex = compatiblePhysicalDevices[PHYSICAL_DEVICE_CHOICE_INDEX];

		_device.physicalDevice = &_createDeviceInfo.instance->physicalDevices[physicalDeviceIndex];

		_device.queueGroups.resize(physicalDevicesQueueGroups[physicalDeviceIndex].queues.size());
		for (size_t iGroup = 0; iGroup != _device.queueGroups.size(); ++iGroup)
		{
			_device.queueGroups[iGroup].flags = physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].flags;
			_device.queueGroups[iGroup].presentability = physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].presentability;
			_device.queueGroups[iGroup].priority = physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].priority;
			_device.queueGroups[iGroup].familyIndex = physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].familyIndex;
			_device.queueGroups[iGroup].firstIndex = physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].firstIndex;
			_device.queueGroups[iGroup].queues.resize(physicalDevicesQueueGroups[physicalDeviceIndex].queues[iGroup].count);
		}
	}
	else
		return VkS::VKU_NO_COMPATIBLE_PHYSICAL_DEVICE;

	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	for (size_t iGroup = 0; iGroup != _device.queueGroups.size(); ++iGroup)
	{
		// Find correct iInfo
		size_t iInfo = 0;
		bool found = false;
		for (; iInfo != deviceQueueCreateInfos.size(); ++iInfo)
		{
			if (deviceQueueCreateInfos[iInfo].queueFamilyIndex == _device.queueGroups[iGroup].familyIndex)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			deviceQueueCreateInfos.push_back({});
			deviceQueueCreateInfos[iInfo].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfos[iInfo].pNext = nullptr;
			deviceQueueCreateInfos[iInfo].flags = VK_RESERVED_FOR_FUTURE_USE;

			deviceQueueCreateInfos[iInfo].queueFamilyIndex = _device.queueGroups[iInfo].familyIndex;
			deviceQueueCreateInfos[iInfo].queueCount = (uint32_t)_device.queueGroups[iInfo].queues.size();
			deviceQueueCreateInfos[iInfo].pQueuePriorities = &_device.queueGroups[iInfo].priority;
		}
		else // cannot repeat family index
			deviceQueueCreateInfos[iInfo].queueCount += (uint32_t)_device.queueGroups[iInfo].queues.size();
	}

	/// Device
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = (uint32_t)_createDeviceInfo.deviceLayerNames.size();
	deviceCreateInfo.ppEnabledLayerNames = _createDeviceInfo.deviceLayerNames.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)_createDeviceInfo.deviceExtensionNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = _createDeviceInfo.deviceExtensionNames.data();
	deviceCreateInfo.pEnabledFeatures = &_createDeviceInfo.requestedFeatures;

	VkU::vkApiResult = vkCreateDevice(_device.physicalDevice->handle, &deviceCreateInfo, nullptr, &_device.handle);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:						break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_INITIALIZATION_FAILED:	return VkS::VKU_BAD_IMPLEMENTATION;
	case VK_ERROR_LAYER_NOT_PRESENT:		return VkS::VKU_MISSING_LAYER;
	case VK_ERROR_EXTENSION_NOT_PRESENT:	return VkS::VKU_MISSING_EXTENSION;
	case VK_ERROR_FEATURE_NOT_PRESENT:		return VkS::VKU_MISSING_FEATURE;
	case VK_ERROR_TOO_MANY_OBJECTS:			return VkS::VKU_TOO_MANY_OBJECTS;
	case VK_ERROR_DEVICE_LOST:				return VkS::VKU_DEVICE_LOST;
	default:								vkuResult = VkS::VKU_UNKNOWN; break;
	}

	/// Queues
	for (size_t iGroup = 0; iGroup != _device.queueGroups.size(); ++iGroup)
	{
		_device.queueGroups[iGroup].queues.resize(_device.queueGroups[iGroup].queues.size());
		for (size_t iQueue = 0; iQueue != _device.queueGroups[iGroup].queues.size(); ++iQueue)
		{
			uint32_t queueFamily = _device.queueGroups[iGroup].familyIndex;
			uint32_t queueIndex = _device.queueGroups[iGroup].firstIndex + (uint32_t)iQueue;
			vkGetDeviceQueue(_device.handle, queueFamily, queueIndex, &_device.queueGroups[iGroup].queues[iQueue]);
		}
	}

	/// Windows properties
	for (size_t iWindow = 0; iWindow != _device.windows.size(); ++iWindow)
	{
		// Surface Format
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		uint32_t propertyCount = 0;
		VkU::vkApiResult = vkGetPhysicalDeviceSurfaceFormatsKHR(_device.physicalDevice->handle, _device.windows[iWindow]->surface, &propertyCount, nullptr);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_INCOMPLETE:						vkuResult = VkS::VKU_INCOMPLETE;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
		surfaceFormats.resize(propertyCount);
		VkU::vkApiResult = vkGetPhysicalDeviceSurfaceFormatsKHR(_device.physicalDevice->handle, _device.windows[iWindow]->surface, &propertyCount, surfaceFormats.data());
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_INCOMPLETE:						vkuResult = VkS::VKU_INCOMPLETE;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}

		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			_device.windows[iWindow]->colorFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		else
			_device.windows[iWindow]->colorFormat = surfaceFormats[0];

		// Composite Alpha
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VkU::vkApiResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device.physicalDevice->handle, _device.windows[iWindow]->surface, &surfaceCapabilities);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}

		_device.windows[iWindow]->compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		for (size_t iAlpha = 0; iAlpha != _createDeviceInfo.windowsProperties[iWindow].preferedCompositeAlphas.size(); ++iAlpha)
		{
			if ((_createDeviceInfo.windowsProperties[iWindow].preferedCompositeAlphas[iAlpha] & surfaceCapabilities.supportedCompositeAlpha) == _createDeviceInfo.windowsProperties[iWindow].preferedCompositeAlphas[iAlpha])
			{
				_device.windows[iWindow]->compositeAlpha = _createDeviceInfo.windowsProperties[iWindow].preferedCompositeAlphas[iAlpha];
				break;
			}
		}

		// Present Mode
		std::vector<VkPresentModeKHR> presentModes;
		propertyCount = 0;
		VkU::vkApiResult = vkGetPhysicalDeviceSurfacePresentModesKHR(_device.physicalDevice->handle, _device.windows[iWindow]->surface, &propertyCount, nullptr);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
		presentModes.resize(propertyCount);
		VkU::vkApiResult = vkGetPhysicalDeviceSurfacePresentModesKHR(_device.physicalDevice->handle, _device.windows[iWindow]->surface, &propertyCount, presentModes.data());
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}

		_device.windows[iWindow]->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (size_t iPrefered = 0; iPrefered != _createDeviceInfo.windowsProperties[iWindow].preferedPresentModes.size(); ++iPrefered)
		{
			for (size_t iMode = 0; iMode != presentModes.size(); ++iMode)
			{
				if (presentModes[iMode] == _createDeviceInfo.windowsProperties[iWindow].preferedPresentModes[iPrefered])
				{
					_device.windows[iWindow]->presentMode = _createDeviceInfo.windowsProperties[iWindow].preferedPresentModes[iPrefered];
					iPrefered = _createDeviceInfo.windowsProperties[iWindow].preferedPresentModes.size() - 1;
					break;
				}
			}
		}
	}

	return vkuResult;
}

VkS::VKU_RESULT VkA::CreateWindowResources(VkS::Window& _window, VkU::CreateWindowResourcesInfo _createWindowResourcesInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	uint32_t propertyCount;

	// swapchain
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VkU::vkApiResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_createWindowResourcesInfo.physicalDevice->handle, _window.surface, &surfaceCapabilities);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}

		// Set Swapchain size
		_window.extent.width = surfaceCapabilities.currentExtent.width;
		_window.extent.height = surfaceCapabilities.currentExtent.height;

		// Cap Swapchain width
		if (_window.extent.width > surfaceCapabilities.maxImageExtent.width)
			_window.extent.width = surfaceCapabilities.maxImageExtent.width;
		else if (_window.extent.width < surfaceCapabilities.minImageExtent.width)
			_window.extent.width = surfaceCapabilities.minImageExtent.width;

		// Cam Swapchain height
		if (_window.extent.height > surfaceCapabilities.maxImageExtent.height)
			_window.extent.height = surfaceCapabilities.maxImageExtent.height;
		else if (_window.extent.height < surfaceCapabilities.minImageExtent.height)
			_window.extent.height = surfaceCapabilities.minImageExtent.height;

		// Cap image count
		if (_createWindowResourcesInfo.imageCount > surfaceCapabilities.maxImageCount)
			_createWindowResourcesInfo.imageCount = surfaceCapabilities.maxImageCount;
		else if (_createWindowResourcesInfo.imageCount < surfaceCapabilities.minImageCount)
			_createWindowResourcesInfo.imageCount = surfaceCapabilities.minImageCount;

		// Swapchain creation structure
		VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
		swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfoKHR.pNext = nullptr;
		swapchainCreateInfoKHR.flags = VK_RESERVED_FOR_FUTURE_USE;
		swapchainCreateInfoKHR.surface = _window.surface;
		swapchainCreateInfoKHR.minImageCount = _createWindowResourcesInfo.imageCount;
		swapchainCreateInfoKHR.imageFormat = _window.colorFormat.format;
		swapchainCreateInfoKHR.imageColorSpace = _window.colorFormat.colorSpace;
		swapchainCreateInfoKHR.imageExtent = _window.extent;
		swapchainCreateInfoKHR.imageArrayLayers = 1;
		swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
		swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;
		swapchainCreateInfoKHR.preTransform = surfaceCapabilities.currentTransform;
		swapchainCreateInfoKHR.compositeAlpha = _window.compositeAlpha;
		swapchainCreateInfoKHR.presentMode = _window.presentMode;
		swapchainCreateInfoKHR.clipped = VK_TRUE;
		swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;
		VkU::vkApiResult = vkCreateSwapchainKHR(_createWindowResourcesInfo.vkDevice, &swapchainCreateInfoKHR, nullptr, &_window.swapchain);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:				return VkS::VKU_DEVICE_LOST;
		case VK_ERROR_SURFACE_LOST_KHR:			return VkS::VKU_SURFACE_LOST;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:	return VkS::VKU_NATIVE_WINDOW_IN_USE;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	// images
	{
		propertyCount = 0;
		VkU::vkApiResult = vkGetSwapchainImagesKHR(_createWindowResourcesInfo.vkDevice, _window.swapchain, &propertyCount, nullptr);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
		_window.images.resize(propertyCount);
		VkU::vkApiResult = vkGetSwapchainImagesKHR(_createWindowResourcesInfo.vkDevice, _window.swapchain, &propertyCount, _window.images.data());
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:						break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
		default:								vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	// views
	{
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		imageViewCreateInfo.image = VK_NULL_HANDLE;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _window.colorFormat.format;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		_window.views.resize(_window.images.size());
		for (size_t iView = 0; iView != _window.views.size(); ++iView)
		{
			imageViewCreateInfo.image = _window.images[iView];
			VkU::vkApiResult = vkCreateImageView(_createWindowResourcesInfo.vkDevice, &imageViewCreateInfo, nullptr, &_window.views[iView]);
			switch (VkU::vkApiResult)
			{
			case VK_SUCCESS:						break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:		return VkS::VKU_NO_HOST_MEMORY;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:		return VkS::VKU_NO_DEVICE_MEMORY;
			default:								vkuResult = VkS::VKU_UNKNOWN; break;
			}
		}
	}

	// depth buffer
	if (_window.depthImage != nullptr)
	{
		// Depth Image
		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = _createWindowResourcesInfo.physicalDevice->depthFormat;
		imageCreateInfo.extent = { _window.extent.width, _window.extent.height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		VkU::vkApiResult = vkCreateImage(_createWindowResourcesInfo.vkDevice, &imageCreateInfo, nullptr, &_window.depthImage->handle);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		// Depth Memory
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(_createWindowResourcesInfo.vkDevice, _window.depthImage->handle, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = -1;

		for (uint32_t i = 0; i != _createWindowResourcesInfo.physicalDevice->memoryProperties.memoryTypeCount; ++i)
		{
			if ((memoryRequirements.memoryTypeBits & (1 << i)) && (_createWindowResourcesInfo.physicalDevice->memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				memoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}

		VkU::vkApiResult = vkAllocateMemory(_createWindowResourcesInfo.vkDevice, &memoryAllocateInfo, nullptr, &_window.depthImage->memory);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:							break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:			return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:			return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_TOO_MANY_OBJECTS:				return VkS::VKU_TOO_MANY_OBJECTS;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:	return VkS::VKU_INVALID_EXTERNAL_HANDLE;
		default:									vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkU::vkApiResult = vkBindImageMemory(_createWindowResourcesInfo.vkDevice, _window.depthImage->handle, _window.depthImage->memory, 0);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		// Depth View
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		imageViewCreateInfo.image = _window.depthImage->handle;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _createWindowResourcesInfo.physicalDevice->depthFormat;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		VkU::vkApiResult = vkCreateImageView(_createWindowResourcesInfo.vkDevice, &imageViewCreateInfo, nullptr, &_window.depthImage->view);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkU::vkApiResult = vkWaitForFences(_createWindowResourcesInfo.vkDevice, 1, &_createWindowResourcesInfo.setupFence, VK_TRUE, ~0U);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_TIMEOUT:					vkuResult = VkS::VKU_TIMEOUT;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkU::vkApiResult = vkResetFences(_createWindowResourcesInfo.vkDevice, 1, &_createWindowResourcesInfo.setupFence);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		VkU::vkApiResult = vkBeginCommandBuffer(_createWindowResourcesInfo.setupCommandBuffer, &commandBufferBeginInfo);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = _window.depthImage->handle;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(_createWindowResourcesInfo.setupCommandBuffer, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		//vkCmdPipelineBarrier(_swapchainResourcesInfo.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		VkU::vkApiResult = vkEndCommandBuffer(_createWindowResourcesInfo.setupCommandBuffer);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_createWindowResourcesInfo.setupCommandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		VkU::vkApiResult = vkQueueSubmit(_createWindowResourcesInfo.setupQueue, 1, &submitInfo, _createWindowResourcesInfo.setupFence);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	// framebuffers
	{}
	{
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		framebufferCreateInfo.renderPass = _createWindowResourcesInfo.renderPass;
		framebufferCreateInfo.width = _window.extent.width;
		framebufferCreateInfo.height = _window.extent.height;
		framebufferCreateInfo.layers = 1;

		_window.framebuffers.resize(_window.images.size());
		for (size_t iFramebuffer = 0; iFramebuffer != _window.framebuffers.size(); ++iFramebuffer)
		{
			std::vector<VkImageView> attachments;

			if (_window.depthImage != nullptr)
				attachments = { _window.views[iFramebuffer], _window.depthImage->view, };
			else
				attachments = { _window.views[iFramebuffer], };

			framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();

			VkU::vkApiResult = vkCreateFramebuffer(_createWindowResourcesInfo.vkDevice, &framebufferCreateInfo, nullptr, &_window.framebuffers[iFramebuffer]);
			switch (VkU::vkApiResult)
			{
			case VK_SUCCESS:					break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
			default:							vkuResult = VkS::VKU_UNKNOWN; break;
			}
		}
	}

	// render fences
	{
		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		_window.fences.resize(_window.images.size());
		for (size_t iFence = 0; iFence != _window.fences.size(); ++iFence)
		{
			VkU::vkApiResult = vkCreateFence(_createWindowResourcesInfo.vkDevice, &fenceCreateInfo, nullptr, &_window.fences[iFence]);
			switch (VkU::vkApiResult)
			{
			case VK_SUCCESS:					break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
			default:							vkuResult = VkS::VKU_UNKNOWN; break;
			}
		}
	}

	// render semaphores
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

		VkU::vkApiResult = vkCreateSemaphore(_createWindowResourcesInfo.vkDevice, &semaphoreCreateInfo, nullptr, &_window.imageAvailableSemaphore);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN;
		}

		VkU::vkApiResult = vkCreateSemaphore(_createWindowResourcesInfo.vkDevice, &semaphoreCreateInfo, nullptr, &_window.renderDoneSemaphore);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN;
		}
	}

	// render commandBuffers
	{
		_window.secondaryCommandBuffers.resize(_window.images.size());

		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = _createWindowResourcesInfo.graphicsCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		commandBufferAllocateInfo.commandBufferCount = (uint32_t)_window.secondaryCommandBuffers.size();

		VkU::vkApiResult = vkAllocateCommandBuffers(_createWindowResourcesInfo.vkDevice, &commandBufferAllocateInfo, _window.secondaryCommandBuffers.data());
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	return vkuResult;
}
VkS::VKU_RESULT VkA::CreateShaderModule(VkShaderModule & _shaderModule, VkU::CreateShaderModuleInfo _createShaderModuleInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	size_t fileSize = 0;
	char* buffer = nullptr;

	std::ifstream file(_createShaderModuleInfo.filename, std::ios::ate | std::ios::binary);

	if (file.good())
	{
		fileSize = (size_t)file.tellg();
		file.seekg(0);

		if (file.good())
		{
			buffer = new char[fileSize];
			file.read(buffer, fileSize);
		}
		else
		{
			fileSize = 0;
		}
	}
	else
		return VkS::VKU_SHADER_FILE_NOT_OPENED;

	file.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = (const uint32_t*)buffer;

	VkU::vkApiResult = vkCreateShaderModule(_createShaderModuleInfo.vkDevice, &shaderModuleCreateInfo, nullptr, &_shaderModule);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	vkuResult = VkS::VKU_NO_HOST_MEMORY; break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	vkuResult = VkS::VKU_NO_DEVICE_MEMORY; break;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	delete[] buffer;

	if (vkuResult > VkS::VKU_LARGEST_SUCCESS_VALUE)
		return vkuResult;

	return vkuResult;
}

VkS::VKU_RESULT VkA::CreateBuffer(VkS::Buffer & _buffer, VkU::CreateBufferInfo _createBufferInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	_buffer.size = _createBufferInfo.size;

	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = VK_FLAGS_NONE;
	bufferCreateInfo.size = _buffer.size;
	bufferCreateInfo.usage = _createBufferInfo.bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VkU::vkApiResult = vkCreateBuffer(_createBufferInfo.vkDevice, &bufferCreateInfo, nullptr, &_buffer.handle);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_createBufferInfo.vkDevice, _buffer.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = -1;

	for (uint32_t i = 0; i != _createBufferInfo.physicalDevice->memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (_createBufferInfo.physicalDevice->memoryProperties.memoryTypes[i].propertyFlags & _createBufferInfo.memoryPropertyFlags) == _createBufferInfo.memoryPropertyFlags)
		{
			memoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	VkU::vkApiResult = vkAllocateMemory(_createBufferInfo.vkDevice, &memoryAllocateInfo, nullptr, &_buffer.memory);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_TOO_MANY_OBJECTS:		return VkS::VKU_TOO_MANY_OBJECTS;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkU::vkApiResult = vkBindBufferMemory(_createBufferInfo.vkDevice, _buffer.handle, _buffer.memory, 0);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	return vkuResult;
}
VkS::VKU_RESULT VkA::FillBuffer(VkU::FillBufferInfo _fillBufferInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	void* data;
	VkU::vkApiResult = vkMapMemory(_fillBufferInfo.vkDevice, _fillBufferInfo.dstBuffer.memory, 0, _fillBufferInfo.size, 0, &data);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_MEMORY_MAP_FAILED:	return VkS::VKU_MEMORY_MAP_FAILED;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}
	memcpy(data, _fillBufferInfo.data, _fillBufferInfo.size);
	vkUnmapMemory(_fillBufferInfo.vkDevice, _fillBufferInfo.dstBuffer.memory);

	return vkuResult;
}
VkS::VKU_RESULT VkA::FillBuffer2(VkU::FillBufferInfo2 _fillBufferInfo2)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	void* data;
	VkU::vkApiResult = vkMapMemory(_fillBufferInfo2.vkDevice, _fillBufferInfo2.dstBuffer.memory, _fillBufferInfo2.targetBufferMemoryOffset, _fillBufferInfo2.targetBufferMemorySize, 0, &data);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_MEMORY_MAP_FAILED:	return VkS::VKU_MEMORY_MAP_FAILED;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	for (size_t iRegion = 0; iRegion != _fillBufferInfo2.memoryFillRegions.size(); ++iRegion)
	{
		memcpy((void*)(((char*)data) + _fillBufferInfo2.memoryFillRegions[iRegion].targetBufferMemoryOffset), _fillBufferInfo2.memoryFillRegions[iRegion].data, _fillBufferInfo2.memoryFillRegions[iRegion].size);
	}

	vkUnmapMemory(_fillBufferInfo2.vkDevice, _fillBufferInfo2.dstBuffer.memory);

	return vkuResult;
}
VkS::VKU_RESULT VkA::CopyBuffer(VkU::CopyBufferInfo _copyBufferInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	VkU::vkApiResult = vkWaitForFences(_copyBufferInfo.vkDevice, 1, &_copyBufferInfo.setupFence, VK_TRUE, ~0U);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_TIMEOUT:					return VkS::VKU_TIMEOUT;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkU::vkApiResult = vkResetFences(_copyBufferInfo.vkDevice, 1, &_copyBufferInfo.setupFence);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	VkU::vkApiResult = vkBeginCommandBuffer(_copyBufferInfo.setupCommandBuffer, &commandBufferBeginInfo);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	vkCmdCopyBuffer(_copyBufferInfo.setupCommandBuffer, _copyBufferInfo.srcBuffer, _copyBufferInfo.dstBuffer, (uint32_t)_copyBufferInfo.copyRegions.size(), _copyBufferInfo.copyRegions.data());

	VkU::vkApiResult = vkEndCommandBuffer(_copyBufferInfo.setupCommandBuffer);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = (uint32_t)_copyBufferInfo.waitSemaphores.size();
	submitInfo.pWaitSemaphores = _copyBufferInfo.waitSemaphores.data();
	submitInfo.pWaitDstStageMask = _copyBufferInfo.waitStageFlags.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_copyBufferInfo.setupCommandBuffer;
	submitInfo.signalSemaphoreCount = (uint32_t)_copyBufferInfo.signalSemaphores.size();
	submitInfo.pSignalSemaphores = _copyBufferInfo.signalSemaphores.data();
	VkU::vkApiResult = vkQueueSubmit(_copyBufferInfo.setupQueue, 1, &submitInfo, _copyBufferInfo.setupFence);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	return vkuResult;
}
void VkA::CopyBuffers(VkU::CopyBuffersInfo _copyBuffersInfo)
{
	for(size_t iRegion = 0; iRegion != _copyBuffersInfo.bufferRegion.size(); ++iRegion)
	{
		vkCmdCopyBuffer(_copyBuffersInfo.commandBuffer, _copyBuffersInfo.srcBuffer, _copyBuffersInfo.bufferRegion[iRegion].dstBuffer, (uint32_t)_copyBuffersInfo.bufferRegion[iRegion].copyRegions.size(), _copyBuffersInfo.bufferRegion[iRegion].copyRegions.data());
	}
}

VkS::VKU_RESULT VkA::LoadImageData(VkS::ImageData& _imageData, VkU::LoadImageDataInfo _loadImageDataInfo)
{
	if (_imageData.data != nullptr)
		delete[] _imageData.data;
	_imageData.data = nullptr;
	_imageData.extent2D.width = 0;
	_imageData.extent2D.height = 0;
	_imageData.format = VK_FORMAT_UNDEFINED;

	FILE* fTGA;
	fTGA = fopen(_loadImageDataInfo.fileName, "rb");

	if (fTGA == NULL)
		return VkS::VKU_IMAGE_FILE_NOT_OPENED;

	uint8_t header[12];
	if (fread(&header, sizeof(header), 1, fTGA) == 0)
		return VkS::VKU_IMAGE_FILE_READING_FAILURE;

	//uint8_t compressedTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };
	uint8_t uncompressedTGAcompare[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };

	// Get data
	if (memcmp(uncompressedTGAcompare, &header, sizeof(header)) == 0)
	{
		uint8_t header[6];

		if (fread(header, sizeof(header), 1, fTGA) == 0)
			return VkS::VKU_IMAGE_FILE_READING_FAILURE;

		_imageData.extent2D.width = header[1] * 256 + header[0];
		_imageData.extent2D.height = header[3] * 256 + header[2];
		uint8_t bpp = header[4];

		if (_imageData.extent2D.width == 0 || _imageData.extent2D.height == 0 || (bpp != 24 && bpp != 32))
			return VkS::VKU_IMAGE_FILE_UNEXPECTED_FORMAT;

		_imageData.size = _imageData.extent2D.width * _imageData.extent2D.height;
		if (bpp == 24)
		{
			_imageData.format = VK_FORMAT_B8G8R8_UNORM;
			_imageData.size *= 3;
		}
		else if (bpp == 32)
		{
			_imageData.format = VK_FORMAT_B8G8R8A8_UNORM;
			_imageData.size *= 4;
		}

		_imageData.data = new uint8_t[_imageData.size];
		if (fread(_imageData.data, 1, _imageData.size, fTGA) != _imageData.size)
		{
			delete[] _imageData.data;
			_imageData.data = nullptr;
			return VkS::VKU_IMAGE_FILE_READING_FAILURE;
		}
	}
	else return VkS::VKU_IMAGE_FILE_NOT_UNCOMPRESSED_TGA;

	return VkS::VKU_SUCCESS;
}
VkS::VKU_RESULT VkA::CreateTexture2(VkS::Texture& _texture, VkU::CreateTextureInfo2 _createTextureInfo2)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	// create
	{
		_texture.extent.width = _createTextureInfo2.textureData->extent2D.width;
		_texture.extent.height = _createTextureInfo2.textureData->extent2D.height;

		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = _createTextureInfo2.textureData->format;
		imageCreateInfo.extent = { _createTextureInfo2.textureData->extent2D.width, _createTextureInfo2.textureData->extent2D.height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkU::vkApiResult = vkCreateImage(_createTextureInfo2.vkDevice, &imageCreateInfo, nullptr, &_texture.handle);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(_createTextureInfo2.vkDevice, _texture.handle, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = VkA::FindMemoryTypeIndex(_createTextureInfo2.physicalDevice, memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkU::vkApiResult = vkAllocateMemory(_createTextureInfo2.vkDevice, &memoryAllocateInfo, nullptr, &_texture.memory);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_TOO_MANY_OBJECTS:		return VkS::VKU_TOO_MANY_OBJECTS;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkU::vkApiResult = vkBindImageMemory(_createTextureInfo2.vkDevice, _texture.handle, _texture.memory, 0);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		imageViewCreateInfo.image = _texture.handle;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _createTextureInfo2.textureData->format;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		VkU::vkApiResult = vkCreateImageView(_createTextureInfo2.vkDevice, &imageViewCreateInfo, nullptr, &_texture.view);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	VkImage stagingTextureHandle;
	VkDeviceMemory stagingTextureMemory;
	// create staging
	{
		VkImageCreateInfo stagingImageCreateInfo;
		stagingImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		stagingImageCreateInfo.pNext = nullptr;
		stagingImageCreateInfo.flags = 0;
		stagingImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		stagingImageCreateInfo.format = _createTextureInfo2.textureData->format;
		stagingImageCreateInfo.extent = { _createTextureInfo2.textureData->extent2D.width, _createTextureInfo2.textureData->extent2D.height, 1 };
		stagingImageCreateInfo.mipLevels = 1;
		stagingImageCreateInfo.arrayLayers = 1;
		stagingImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		stagingImageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		stagingImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		stagingImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingImageCreateInfo.queueFamilyIndexCount = 0;
		stagingImageCreateInfo.pQueueFamilyIndices = nullptr;
		stagingImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkU::vkApiResult = vkCreateImage(_createTextureInfo2.vkDevice, &stagingImageCreateInfo, nullptr, &stagingTextureHandle);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(_createTextureInfo2.vkDevice, stagingTextureHandle, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = VkA::FindMemoryTypeIndex(_createTextureInfo2.physicalDevice, memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkU::vkApiResult = vkAllocateMemory(_createTextureInfo2.vkDevice, &memoryAllocateInfo, nullptr, &stagingTextureMemory);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_TOO_MANY_OBJECTS:		return VkS::VKU_TOO_MANY_OBJECTS;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkU::vkApiResult = vkBindImageMemory(_createTextureInfo2.vkDevice, stagingTextureHandle, stagingTextureMemory, 0);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	// fill staging
	{
		VkImageSubresource imageSubresource;
		imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresource.mipLevel = 0;
		imageSubresource.arrayLayer = 0;

		VkSubresourceLayout subresourceLayout;
		vkGetImageSubresourceLayout(_createTextureInfo2.vkDevice, stagingTextureHandle, &imageSubresource, &subresourceLayout);

		void* mappedData;
		VkU::vkApiResult = vkMapMemory(_createTextureInfo2.vkDevice, stagingTextureMemory, 0, _createTextureInfo2.textureData->size, 0, &mappedData);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_MEMORY_MAP_FAILED:	return VkS::VKU_MEMORY_MAP_FAILED;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		if (subresourceLayout.rowPitch == _createTextureInfo2.textureData->extent2D.width * 4)
			memcpy(mappedData, _createTextureInfo2.textureData->data, _createTextureInfo2.textureData->size);
		else
		{
			uint8_t* _data8b = (uint8_t*)_createTextureInfo2.textureData->data;
			uint8_t* data8b = (uint8_t*)mappedData;

			for (uint32_t y = 0; y < _createTextureInfo2.textureData->extent2D.height; y++)
				memcpy(&data8b[y * subresourceLayout.rowPitch], &_data8b[y * _createTextureInfo2.textureData->extent2D.width * 4], _createTextureInfo2.textureData->extent2D.width * 4);
		}

		vkUnmapMemory(_createTextureInfo2.vkDevice, stagingTextureMemory);
	}

	// transfer
	{
		VkU::vkApiResult = vkWaitForFences(_createTextureInfo2.vkDevice, 1, &_createTextureInfo2.setupFence, VK_TRUE, -1);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_TIMEOUT:					return VkS::VKU_TIMEOUT;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
		VkU::vkApiResult = vkResetFences(_createTextureInfo2.vkDevice, 1, &_createTextureInfo2.setupFence);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		VkU::vkApiResult = vkBeginCommandBuffer(_createTextureInfo2.setupCommandBuffer, &commandBufferBeginInfo);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkImageMemoryBarrier sourceImageMemoryBarrier;
		sourceImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		sourceImageMemoryBarrier.pNext = nullptr;
		sourceImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		sourceImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		sourceImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		sourceImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		sourceImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		sourceImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		sourceImageMemoryBarrier.image = stagingTextureHandle;
		sourceImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sourceImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		sourceImageMemoryBarrier.subresourceRange.levelCount = 1;
		sourceImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		sourceImageMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(_createTextureInfo2.setupCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &sourceImageMemoryBarrier);

		VkImageMemoryBarrier dstImageMemoryBarrier;
		dstImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		dstImageMemoryBarrier.pNext = nullptr;
		dstImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		dstImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		dstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		dstImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		dstImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		dstImageMemoryBarrier.image = _texture.handle;
		dstImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		dstImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		dstImageMemoryBarrier.subresourceRange.levelCount = 1;
		dstImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		dstImageMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(_createTextureInfo2.setupCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &dstImageMemoryBarrier);

		VkImageSubresourceLayers imageSubresourceLayers;
		imageSubresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceLayers.mipLevel = 0;
		imageSubresourceLayers.baseArrayLayer = 0;
		imageSubresourceLayers.layerCount = 1;

		VkImageCopy imageCopy;
		imageCopy.srcSubresource = imageSubresourceLayers;
		imageCopy.srcOffset.x = 0;
		imageCopy.srcOffset.y = 0;
		imageCopy.srcOffset.z = 0;
		imageCopy.dstSubresource = imageSubresourceLayers;
		imageCopy.dstOffset.x = 0;
		imageCopy.dstOffset.y = 0;
		imageCopy.dstOffset.z = 0;
		imageCopy.extent.width = _texture.extent.width;
		imageCopy.extent.height = _texture.extent.height;
		imageCopy.extent.depth = 1;

		vkCmdCopyImage(_createTextureInfo2.setupCommandBuffer, stagingTextureHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

		VkImageMemoryBarrier finalMemoryBarrier;
		finalMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		finalMemoryBarrier.pNext = nullptr;
		finalMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		finalMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		finalMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		finalMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		finalMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalMemoryBarrier.image = _texture.handle;
		finalMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		finalMemoryBarrier.subresourceRange.baseMipLevel = 0;
		finalMemoryBarrier.subresourceRange.levelCount = 1;
		finalMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		finalMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(_createTextureInfo2.setupCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalMemoryBarrier);

		VkU::vkApiResult = vkEndCommandBuffer(_createTextureInfo2.setupCommandBuffer);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_createTextureInfo2.setupCommandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		VkU::vkApiResult = vkQueueSubmit(_createTextureInfo2.setupQueue, 1, &submitInfo, _createTextureInfo2.setupFence);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}

		// Wait for textures to be transfered
		VkU::vkApiResult = vkQueueWaitIdle(_createTextureInfo2.setupQueue);
		switch (VkU::vkApiResult)
		{
		case VK_SUCCESS:					break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
		case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
		default:							vkuResult = VkS::VKU_UNKNOWN; break;
		}
	}

	vkDestroyImage(_createTextureInfo2.vkDevice, stagingTextureHandle, nullptr);
	vkFreeMemory(_createTextureInfo2.vkDevice, stagingTextureMemory, nullptr);

	return vkuResult;
}
VkS::VKU_RESULT VkA::CreateTexture(VkS::Texture & _texture, VkU::CreateTextureInfo _createTextureInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	_texture.extent.width = _createTextureInfo.extent3D.width;
	_texture.extent.height = _createTextureInfo.extent3D.height;

	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _createTextureInfo.format;
	imageCreateInfo.extent = _createTextureInfo.extent3D;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageCreateInfo.usage = _createTextureInfo.usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VkU::vkApiResult = vkCreateImage(_createTextureInfo.vkDevice, &imageCreateInfo, nullptr, &_texture.handle);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_createTextureInfo.vkDevice, _texture.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkA::FindMemoryTypeIndex(_createTextureInfo.physicalDevice, memoryRequirements, _createTextureInfo.memoryPropertyFlags);

	VkU::vkApiResult = vkAllocateMemory(_createTextureInfo.vkDevice, &memoryAllocateInfo, nullptr, &_texture.memory);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_TOO_MANY_OBJECTS:		return VkS::VKU_TOO_MANY_OBJECTS;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkU::vkApiResult = vkBindImageMemory(_createTextureInfo.vkDevice, _texture.handle, _texture.memory, 0);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	imageViewCreateInfo.image = _texture.handle;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = _createTextureInfo.format;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = _createTextureInfo.aspect;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	VkU::vkApiResult = vkCreateImageView(_createTextureInfo.vkDevice, &imageViewCreateInfo, nullptr, &_texture.view);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	return vkuResult;
}
VkS::VKU_RESULT VkA::FillTexture(VkU::FillTextureInfo _fillTextureInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	VkImageSubresource imageSubresource;
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.mipLevel = 0;
	imageSubresource.arrayLayer = 0;

	VkSubresourceLayout subresourceLayout;
	vkGetImageSubresourceLayout(_fillTextureInfo.vkDevice, _fillTextureInfo.dstTexture.handle, &imageSubresource, &subresourceLayout);

	void* mappedData;
	VkU::vkApiResult = vkMapMemory(_fillTextureInfo.vkDevice, _fillTextureInfo.dstTexture.memory, 0, _fillTextureInfo.size, 0, &mappedData);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_MEMORY_MAP_FAILED:	return VkS::VKU_MEMORY_MAP_FAILED;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	if (subresourceLayout.rowPitch == _fillTextureInfo.width * 4)
		memcpy(mappedData, _fillTextureInfo.data, _fillTextureInfo.size);
	else
	{
		uint8_t* _data8b = (uint8_t*)_fillTextureInfo.data;
		uint8_t* data8b = (uint8_t*)mappedData;

		for (uint32_t y = 0; y < _fillTextureInfo.height; y++)
			memcpy(&data8b[y * subresourceLayout.rowPitch], &_data8b[y * _fillTextureInfo.width * 4], _fillTextureInfo.width * 4);
	}

	vkUnmapMemory(_fillTextureInfo.vkDevice, _fillTextureInfo.dstTexture.memory);

	return vkuResult;
}
/// TODO: unify memory barrier call
VkS::VKU_RESULT VkA::CopyTexture(VkU::CopyTextureInfo _copyTextureInfo)
{
	VkS::VKU_RESULT vkuResult = VkS::VKU_SUCCESS;

	VkU::vkApiResult = vkWaitForFences(_copyTextureInfo.vkDevice, 1, &_copyTextureInfo.setupFence, VK_TRUE, -1);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_TIMEOUT:					return VkS::VKU_TIMEOUT;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}
	VkU::vkApiResult = vkResetFences(_copyTextureInfo.vkDevice, 1, &_copyTextureInfo.setupFence);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	VkU::vkApiResult = vkBeginCommandBuffer(_copyTextureInfo.setupCommandBuffer, &commandBufferBeginInfo);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkImageMemoryBarrier sourceImageMemoryBarrier;
	sourceImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	sourceImageMemoryBarrier.pNext = nullptr;
	sourceImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	sourceImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	sourceImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	sourceImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	sourceImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sourceImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sourceImageMemoryBarrier.image = _copyTextureInfo.srcTexture.handle;
	sourceImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sourceImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	sourceImageMemoryBarrier.subresourceRange.levelCount = 1;
	sourceImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	sourceImageMemoryBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(_copyTextureInfo.setupCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &sourceImageMemoryBarrier);

	VkImageMemoryBarrier dstImageMemoryBarrier;
	dstImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	dstImageMemoryBarrier.pNext = nullptr;
	dstImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	dstImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	dstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	dstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	dstImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	dstImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	dstImageMemoryBarrier.image = _copyTextureInfo.dstTexture.handle;
	dstImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	dstImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	dstImageMemoryBarrier.subresourceRange.levelCount = 1;
	dstImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	dstImageMemoryBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(_copyTextureInfo.setupCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &dstImageMemoryBarrier);

	VkImageSubresourceLayers imageSubresourceLayers;
	imageSubresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceLayers.mipLevel = 0;
	imageSubresourceLayers.baseArrayLayer = 0;
	imageSubresourceLayers.layerCount = 1;

	VkImageCopy imageCopy;
	imageCopy.srcSubresource = imageSubresourceLayers;
	imageCopy.srcOffset.x = 0;
	imageCopy.srcOffset.y = 0;
	imageCopy.srcOffset.z = 0;
	imageCopy.dstSubresource = imageSubresourceLayers;
	imageCopy.dstOffset.x = 0;
	imageCopy.dstOffset.y = 0;
	imageCopy.dstOffset.z = 0;
	imageCopy.extent.width = _copyTextureInfo.dstTexture.extent.width;
	imageCopy.extent.height = _copyTextureInfo.dstTexture.extent.height;
	imageCopy.extent.depth = 1;

	vkCmdCopyImage(_copyTextureInfo.setupCommandBuffer, _copyTextureInfo.srcTexture.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _copyTextureInfo.dstTexture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

	VkImageMemoryBarrier finalMemoryBarrier;
	finalMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	finalMemoryBarrier.pNext = nullptr;
	finalMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	finalMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	finalMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	finalMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	finalMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	finalMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	finalMemoryBarrier.image = _copyTextureInfo.dstTexture.handle;
	finalMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	finalMemoryBarrier.subresourceRange.baseMipLevel = 0;
	finalMemoryBarrier.subresourceRange.levelCount = 1;
	finalMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	finalMemoryBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(_copyTextureInfo.setupCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalMemoryBarrier);

	VkImageMemoryBarrier sourceResetImageMemoryBarrier;
	sourceResetImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	sourceResetImageMemoryBarrier.pNext = nullptr;
	sourceResetImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	sourceResetImageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	sourceResetImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	sourceResetImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	sourceResetImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sourceResetImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	sourceResetImageMemoryBarrier.image = _copyTextureInfo.srcTexture.handle;
	sourceResetImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sourceResetImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	sourceResetImageMemoryBarrier.subresourceRange.levelCount = 1;
	sourceResetImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	sourceResetImageMemoryBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(_copyTextureInfo.setupCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1, &sourceResetImageMemoryBarrier);

	VkU::vkApiResult = vkEndCommandBuffer(_copyTextureInfo.setupCommandBuffer);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_copyTextureInfo.setupCommandBuffer;
	submitInfo.signalSemaphoreCount = (uint32_t)_copyTextureInfo.signalSemaphores.size();
	submitInfo.pSignalSemaphores = _copyTextureInfo.signalSemaphores.data();
	VkU::vkApiResult = vkQueueSubmit(_copyTextureInfo.setupQueue, 1, &submitInfo, _copyTextureInfo.setupFence);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	// Wait for textures to be transfered
	VkU::vkApiResult = vkQueueWaitIdle(_copyTextureInfo.setupQueue);
	switch (VkU::vkApiResult)
	{
	case VK_SUCCESS:					break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:	return VkS::VKU_NO_HOST_MEMORY;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:	return VkS::VKU_NO_DEVICE_MEMORY;
	case VK_ERROR_DEVICE_LOST:			return VkS::VKU_DEVICE_LOST;
	default:							vkuResult = VkS::VKU_UNKNOWN; break;
	}

	return vkuResult;
}

uint32_t VkD::GetVertexStride(Loader::VERTEX_DATATYPE _vertexDatatype)
{
	uint32_t stride = 0;

	char indexSize = 0;

	if ((_vertexDatatype & Loader::VDT_X) == Loader::VDT_X)
		stride += sizeof(float);
	if ((_vertexDatatype & Loader::VDT_Y) == Loader::VDT_Y)
		stride += sizeof(float);
	if ((_vertexDatatype & Loader::VDT_Z) == Loader::VDT_Z)
		stride += sizeof(float);

	if ((_vertexDatatype & Loader::VDT_UV) == Loader::VDT_UV)
		stride += sizeof(float) * 2;

	if ((_vertexDatatype & Loader::VDT_R) == Loader::VDT_R)
		stride += sizeof(float);
	if ((_vertexDatatype & Loader::VDT_G) == Loader::VDT_G)
		stride += sizeof(float);
	if ((_vertexDatatype & Loader::VDT_B) == Loader::VDT_B)
		stride += sizeof(float);
	if ((_vertexDatatype & Loader::VDT_A) == Loader::VDT_A)
		stride += sizeof(float);

	if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_8) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_8)
		indexSize = sizeof(uint8_t);
	if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_16) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_16)
	{
		if (indexSize > 0)
			return 0;

		indexSize = sizeof(uint16_t);
	}

	if ((_vertexDatatype & Loader::VDT_SKELETON_4_BONE_PER_VERTEX) == Loader::VDT_SKELETON_4_BONE_PER_VERTEX)
		stride += (sizeof(float) + indexSize) * 4;

	if ((_vertexDatatype & Loader::VDT_NORMAL) == Loader::VDT_NORMAL)
		stride += sizeof(float) * 3;
	if ((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT)
		stride += sizeof(float) * 6;

	return stride;
}
std::vector<VkVertexInputAttributeDescription> VkD::GetVertexInputAttributeDescriptions(Loader::VERTEX_DATATYPE _vertexDatatype)
{
	uint32_t attributeCount = 0;
	uint32_t attributeIndex = 0;
	uint32_t attributeOffset = 0;

	if ((_vertexDatatype & Loader::VDT_X) == Loader::VDT_X)
		++attributeCount;

	if ((_vertexDatatype & Loader::VDT_UV) == Loader::VDT_UV)
		++attributeCount;

	if ((_vertexDatatype & Loader::VDT_R) == Loader::VDT_R)
		++attributeCount;

	if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_8) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_8)
		++++attributeCount;
	else if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_16) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_16)
		++++attributeCount;

	if ((_vertexDatatype & Loader::VDT_NORMAL) == Loader::VDT_NORMAL)
		++attributeCount;
	if ((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT)
		++++attributeCount;

	char indexSize = 0;

	if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_8) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_8)
		indexSize = sizeof(uint8_t);
	if ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_16) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_16)
	{
		if (indexSize > 0)
			return {};

		indexSize = sizeof(uint16_t);
	}

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription(attributeCount);

	// position
	if ((_vertexDatatype & Loader::VDT_X) == Loader::VDT_X && (_vertexDatatype & Loader::VDT_Y) == Loader::VDT_Y && (_vertexDatatype & Loader::VDT_Z) == Loader::VDT_Z)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_X) == Loader::VDT_X) * VK_FORMAT_R32_SFLOAT +
			((_vertexDatatype & Loader::VDT_Y) == Loader::VDT_Y) * (VK_FORMAT_R32G32_SFLOAT - VK_FORMAT_R32_SFLOAT) +
			((_vertexDatatype & Loader::VDT_Z) == Loader::VDT_Z) * (VK_FORMAT_R32G32B32_SFLOAT - VK_FORMAT_R32G32_SFLOAT));
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_X) == Loader::VDT_X) * sizeof(float) +
			((_vertexDatatype & Loader::VDT_Y) == Loader::VDT_Y) * sizeof(float) +
			((_vertexDatatype & Loader::VDT_Z) == Loader::VDT_Z) * sizeof(float);
		++attributeIndex;
	}

	// uv
	if ((_vertexDatatype & Loader::VDT_UV) == Loader::VDT_UV)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_UV) == Loader::VDT_UV) * VK_FORMAT_R32G32_SFLOAT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_UV) == Loader::VDT_UV) * sizeof(float) * 2;
		++attributeIndex;
	}

	// color
	if ((_vertexDatatype & Loader::VDT_R) == Loader::VDT_R && (_vertexDatatype & Loader::VDT_G) == Loader::VDT_G && (_vertexDatatype & Loader::VDT_B) == Loader::VDT_B && (_vertexDatatype & Loader::VDT_A) == Loader::VDT_A)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_R) == Loader::VDT_R) * VK_FORMAT_R32_SFLOAT +
			((_vertexDatatype & Loader::VDT_G) == Loader::VDT_G) * (VK_FORMAT_R32G32_SFLOAT - VK_FORMAT_R32_SFLOAT) +
			((_vertexDatatype & Loader::VDT_B) == Loader::VDT_B) * (VK_FORMAT_R32G32B32_SFLOAT - VK_FORMAT_R32G32_SFLOAT) +
			((_vertexDatatype & Loader::VDT_A) == Loader::VDT_A) * (VK_FORMAT_R32G32B32A32_SFLOAT - VK_FORMAT_R32G32B32_SFLOAT));
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_R) == Loader::VDT_R) * sizeof(float) +
			((_vertexDatatype & Loader::VDT_G) == Loader::VDT_G) * sizeof(float) +
			((_vertexDatatype & Loader::VDT_B) == Loader::VDT_B) * sizeof(float) +
			((_vertexDatatype & Loader::VDT_A) == Loader::VDT_A) * sizeof(float);
		++attributeIndex;
	}

	// bone indices
	if (((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_8) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_8) ^ ((_vertexDatatype & Loader::VDT_SKELETON_BONE_INDEX_SIZE_16) == Loader::VDT_SKELETON_BONE_INDEX_SIZE_16))
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			(indexSize == sizeof(uint8_t)) * VK_FORMAT_R8_UINT +
			(indexSize == sizeof(uint16_t)) * VK_FORMAT_R16_UINT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			(indexSize == sizeof(uint8_t)) * sizeof(uint8_t) * 4 +
			(indexSize == sizeof(uint16_t)) * sizeof(uint16_t) * 4;
		++attributeIndex;
	}

	// bone weights
	if ((_vertexDatatype & Loader::VDT_SKELETON_4_BONE_PER_VERTEX) == Loader::VDT_SKELETON_4_BONE_PER_VERTEX)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_SKELETON_4_BONE_PER_VERTEX) == Loader::VDT_SKELETON_4_BONE_PER_VERTEX) * VK_FORMAT_R32G32B32A32_SFLOAT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_SKELETON_4_BONE_PER_VERTEX) == Loader::VDT_SKELETON_4_BONE_PER_VERTEX) * sizeof(float) * 4;
		++attributeIndex;
	}

	// normal
	if ((_vertexDatatype & Loader::VDT_NORMAL) == Loader::VDT_NORMAL)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_NORMAL) == Loader::VDT_NORMAL) * VK_FORMAT_R32G32B32_SFLOAT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_NORMAL) == Loader::VDT_NORMAL) * sizeof(float) * 3;
		++attributeIndex;
	}

	// tangent
	if ((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT)
	{
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT) * VK_FORMAT_R32G32B32_SFLOAT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT) * sizeof(float) * 3;
		++attributeIndex;

		// bitangent
		vertexInputAttributeDescription[attributeIndex].location = attributeIndex;
		vertexInputAttributeDescription[attributeIndex].binding = 0;
		vertexInputAttributeDescription[attributeIndex].format = (VkFormat)(
			((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT) * VK_FORMAT_R32G32B32_SFLOAT);
		vertexInputAttributeDescription[attributeIndex].offset = attributeOffset;
		attributeOffset +=
			((_vertexDatatype & Loader::VDT_TANGENT_BITANGENT) == Loader::VDT_TANGENT_BITANGENT) * sizeof(float) * 3;
		++attributeIndex;
	}

	return vertexInputAttributeDescription;
}
