#include "Headers/Vulkan.h"
#define VMA_IMPLEMENTATION
#include "../../Resource Management/Headers/vk_mem_alloc.h"
#include <string>
#include <iostream>
#include <functional>

void VulkanRender::InitAPI(int width, int height)
{
	globalManager = &GlobalManager::getInstance();
	createInstance();
	createSurface(); 
	enumerateDevices();
	selectDevice();
	createAllocator();
	createSemaphores();
	createCommandPool();
	createCommandBuffers();
	createSwapChain(height, width);
	createRenderTargets();
	createRenderPass();
	createFrameBuffers();

	LoadShader("./Renderer/API/Vulkan Shaders/Compiled/VertShader.spv");
	LoadShader("./Renderer/API/Vulkan Shaders/Compiled/FragShader.spv");
	LoadVertexData("test.obj");
	
	buildPipeline("triangle test");

	globalManager->setCamera(new Camera(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0), swapChainExtent.width, swapChainExtent.height, 0.000000001f, 10000));
	camera = globalManager->getCamera();
}
void VulkanRender::Render()
{
	uint32_t imageIndex;
	VkCommandBuffer cmd;

	preRender(cmd, imageIndex);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at(0).pipeline);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &allocation.at(allocation.size()-1).buffer, &offset);
	Matrix44 weTest;
	vkCmdPushConstants(cmd, pipelines.at(pipelines.size()-1).layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewProjection), &camera->viewProj);
	vkCmdDraw(cmd, allocation.at(allocation.size()-1).vertices.get()->size(), 1, 0, 0);

	postRender(cmd, imageIndex);
}
#ifdef _DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRender::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult VulkanRender::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
				"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


#endif

void VulkanRender::createInstance()
{
	VkResult result;

	#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	debugCreateInfo.pUserData = nullptr;
	#endif

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Orbital Physics";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Orbital Physics";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	#ifdef _DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	#endif
	
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	createInfo.pNext = nullptr;
	std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation"};
	
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	

	result = vkCreateInstance(&createInfo, nullptr, &vkInfo.instance);
	if (result != VK_SUCCESS) {
		MessageBox(NULL, "Failed to create instance!", "Error", MB_OK);
		exit(-1);
	}
	#ifdef _DEBUG
	result = CreateDebugUtilsMessengerEXT(vkInfo.instance, &debugCreateInfo, nullptr, &debug.debugMessenger);
	if (result != VK_SUCCESS) {
		MessageBox(NULL, "Failed to create debug messenger!", "Error", MB_OK);
		exit(-1);
	}
	#endif

}

void VulkanRender::createSurface()
{
	VkResult result;
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = *wndInfo.hWnd.get();
	createInfo.hinstance = *wndInfo.hInstance.get();

	result = vkCreateWin32SurfaceKHR(vkInfo.instance, &createInfo, nullptr, &vkInfo.surface);
	if (result != VK_SUCCESS) {
		MessageBox(NULL, "Failed to create surface!", "Error", MB_OK);
		exit(-1);
	}
}

void VulkanRender::enumerateDevices()
{
	uint32_t numDevices = 0;
	VkResult result;

	result = vkEnumeratePhysicalDevices(vkInfo.instance, &numDevices, nullptr);
	if (numDevices == 0) {
		MessageBox(NULL, "Failed to find GPUs with Vulkan support!", "Error", MB_OK);
		exit(-1);
	}
	vkInfo.physicalDevice.resize(numDevices);
	vkInfo.deviceProperties.resize(numDevices);

	result = vkEnumeratePhysicalDevices(vkInfo.instance, &numDevices, vkInfo.physicalDevice.data());
	if (result != VK_SUCCESS) {
		MessageBox(NULL, "Failed to enumerate physical devices!", "Error", MB_OK);
		exit(-1);
	}
	
	for (uint32_t i = 0; i < numDevices; ++i)
	{
		VkPhysicalDevice device = vkInfo.physicalDevice[i];

		#ifdef _DEBUG
		vkGetPhysicalDeviceProperties(device, &vkInfo.deviceProperties[i]);
		#endif

		{
			uint32_t numQueues = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueues, nullptr);
			if (numQueues == 0)
			{
				MessageBox(NULL, "Failed to find queues with Vulkan support!", "Error", MB_OK);
				exit(-1);
			}
			vkInfo.queueFamilyProperties.resize(numQueues);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueues, vkInfo.queueFamilyProperties.data());
		}

		{
			uint32_t numExtensions = 0;
			result = vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to enumerate device extensions!", "Error", MB_OK);
				exit(-1);
			}

			vkInfo.deviceExtensions.resize(numExtensions);
			result = vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, vkInfo.deviceExtensions.data());
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to enumerate device extensions!", "Error", MB_OK);
				exit(-1);
			}
		}
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkInfo.surface, &vkInfo.surfaceCapabilities);

		{
			uint32_t numFormats;
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkInfo.surface, &numFormats, nullptr);
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to get surface formats!", "Error", MB_OK);
				exit(-1);
			}

			surfaceFormats.resize(numFormats);
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkInfo.surface, &numFormats, surfaceFormats.data());
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to get surface formats!", "Error", MB_OK);
				exit(-1);
			}

		}

		{
			uint32_t numPresentModes;
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkInfo.surface, &numPresentModes, nullptr);
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to get surface present modes!", "Error", MB_OK);
				exit(-1);
			}

			presentModes.resize(numPresentModes);
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkInfo.surface, &numPresentModes, presentModes.data());
			if (result != VK_SUCCESS)
			{
				MessageBox(NULL, "Failed to get surface present modes!", "Error", MB_OK);
				exit(-1);
			}

		}

		//vkGetPhysicalDeviceMemoryProperties( device, vkInfo.deviceMemoryProperties.data());
		//vkGetPhysicalDeviceProperties(device, vkInfo.deviceProperties.data());
	}
}

void VulkanRender::selectDevice()
{
	uint32_t extensionsAmount = 0;
	uint32_t mostFeatures = -1;
	int selectedDevice = -1;
	int graphicsId = -1;
	for (int i = 0; i < vkInfo.physicalDevice.size(); ++i)
	{
		VkPhysicalDeviceProperties deviceProp = vkInfo.deviceProperties[i];
		uint32_t numExtensions = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
		std::vector<VkExtensionProperties> extensions(numExtensions);

		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data());
		VkBool32 supportsPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkInfo.physicalDevice[i], 0, vkInfo.surface, &supportsPresent);
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(vkInfo.physicalDevice[i], &features);
		
		for (const auto extension : vkInfo.deviceExtensions)
		{
			for (const auto ext : extensions)
			{
				if (strcmp(extension.extensionName, ext.extensionName) == 0)
				{
					continue;
				}
			}
			if (presentModes.size() == 0)
			{
				continue;
			}
			if (surfaceFormats.size() == 0)
			{
				continue;
			}
		}
		for (int j = 0; j < vkInfo.queueFamilyProperties.size(); ++j)
		{
			std::vector<VkQueueFamilyProperties> props;
			vkGetPhysicalDeviceQueueFamilyProperties(vkInfo.physicalDevice[i], &numExtensions, nullptr);
			props.resize(numExtensions);
			vkGetPhysicalDeviceQueueFamilyProperties(vkInfo.physicalDevice[i], &numExtensions, props.data());

			if (props[j].queueCount == 0)
			{
				continue;
			}
			if ((props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				if (supportsPresent)
				{
					graphicsId = j;
					if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
						selectedDevice = i;
						break;
					}
				}
				
			}
		}
	}

	if (selectedDevice >= 0)
	{
		//This is our GPU! Now lets break it. Turns out this is a bad place to put this. there is a risk the integrated GPU gets picked.

		VkResult result;
		gpuInfo.physicalDevice = vkInfo.physicalDevice[selectedDevice];
		uint32_t num = 0;
		//Get Device Properties
		vkGetPhysicalDeviceProperties(gpuInfo.physicalDevice, &gpuInfo.properties);
		//Get Device Features
		vkGetPhysicalDeviceFeatures(gpuInfo.physicalDevice, &gpuInfo.features);
		//Get Device Memory Properties
		vkGetPhysicalDeviceMemoryProperties(gpuInfo.physicalDevice, &gpuInfo.memoryProperties);
		//Add queue family properties to gpuInfo
		vkGetPhysicalDeviceQueueFamilyProperties(gpuInfo.physicalDevice, &num, nullptr);
		gpuInfo.queueFamilyProperties.resize(num);
		vkGetPhysicalDeviceQueueFamilyProperties(gpuInfo.physicalDevice, &num, gpuInfo.queueFamilyProperties.data());
		//Get Extension Properties
		result = vkEnumerateDeviceExtensionProperties(gpuInfo.physicalDevice, nullptr, &num, nullptr);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to enumerate device extensions!", "Error", MB_OK);
			exit(-1);
		}
		gpuInfo.extensionProperties.resize(num);
		result = vkEnumerateDeviceExtensionProperties(gpuInfo.physicalDevice, nullptr, &num, gpuInfo.extensionProperties.data());
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to enumerate device extensions!", "Error", MB_OK);
			exit(-1);
		}
		//Get Layer Properties
		result = vkEnumerateDeviceLayerProperties(gpuInfo.physicalDevice, &num, nullptr);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to enumerate device layers!", "Error", MB_OK);
			exit(-1);
		}
		gpuInfo.layerProperties.resize(num);
		result = vkEnumerateDeviceLayerProperties(gpuInfo.physicalDevice, &num, gpuInfo.layerProperties.data());
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to enumerate device layers!", "Error", MB_OK);
			exit(-1);
		}
		//Get Surface Capabilities
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuInfo.physicalDevice, vkInfo.surface, &vkInfo.surfaceCapabilities);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to get surface capabilities!", "Error", MB_OK);
			exit(-1);
		}
		//Get Surface Formats
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpuInfo.physicalDevice, vkInfo.surface, &num, nullptr);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to get surface formats!", "Error", MB_OK);
			exit(-1);
		}
		gpuInfo.surfaceFormats.resize(num);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpuInfo.physicalDevice, vkInfo.surface, &num, gpuInfo.surfaceFormats.data());
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to get surface formats!", "Error", MB_OK);
			exit(-1);
		}
		//Get Surface Present Modes
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpuInfo.physicalDevice, vkInfo.surface, &num, nullptr);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to get surface present modes!", "Error", MB_OK);
			exit(-1);
		}
		gpuInfo.presentModes.resize(num);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpuInfo.physicalDevice, vkInfo.surface, &num, gpuInfo.presentModes.data());
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to get surface present modes!", "Error", MB_OK);
			exit(-1);
		}
		createLogicalDevice(graphicsId);
		return;
	}

	MessageBox(NULL, "Failed to find suitable GPU!", "Error", MB_OK);
	exit(-1);
}

void VulkanRender::createLogicalDevice(int graphicsId)
{
	float queuePriority = 1.0f;
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
	//For my sanity graphics and presents use same queue
	queueCreateInfos.resize(1);
	queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[0].pNext = nullptr;
	queueCreateInfos[0].queueFamilyIndex = graphicsId;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = &queuePriority;


	VkPhysicalDeviceFeatures deviceFeatures = {};

	deviceFeatures.textureCompressionBC = VK_TRUE;
	deviceFeatures.imageCubeArray = VK_TRUE;
	deviceFeatures.depthClamp = VK_TRUE;
	deviceFeatures.depthBiasClamp = VK_TRUE;
	deviceFeatures.depthBounds = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	std::vector<const char*> cextensions = { "VK_KHR_swapchain"};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &gpuInfo.features;
	deviceCreateInfo.enabledExtensionCount = cextensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = cextensions.data();

	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;

	std::vector<const char*> validationLayers = {};
	if (enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = validationLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	
	VkResult result;
	result = vkCreateDevice(gpuInfo.physicalDevice, &deviceCreateInfo, nullptr, &vkInfo.device);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create logical device!", "Error", MB_OK);
		exit(-1);
	}

	//Graphics and present queue are 1 and 2 for my sanity.
	vkGetDeviceQueue(vkInfo.device, graphicsId, 0, &graphicsQueue);

}

void VulkanRender::createAllocator()
{
	VkResult result;

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.flags = 0;
	allocatorInfo.physicalDevice = gpuInfo.physicalDevice;
	allocatorInfo.device = vkInfo.device;
	allocatorInfo.instance = vkInfo.instance;
	allocatorInfo.pHeapSizeLimit = nullptr;
	allocatorInfo.pVulkanFunctions = nullptr;
	
	result = vmaCreateAllocator(&allocatorInfo, &vkInfo.allocator);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create allocator!", "Error", MB_OK);
		exit(-1);
	}

}

void VulkanRender::createSemaphores()
{
	VkResult result;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;

	imageAvailableSemaphores.resize(2);
	renderFinishedSemaphores.resize(2);
	for (int i = 0; i < 2; ++i) 
	{
		result = vkCreateSemaphore(vkInfo.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to create semaphore!", "Error", MB_OK);
			exit(-1);
		}
		result = vkCreateSemaphore(vkInfo.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to create semaphore!", "Error", MB_OK);
			exit(-1);
		}
	}
}

void VulkanRender::createCommandPool()
{
	VkResult result;
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.queueFamilyIndex = 0;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	result = vkCreateCommandPool(vkInfo.device, &commandPoolCreateInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create command pool!", "Error", MB_OK);
		exit(-1);
	}

}

void VulkanRender::createCommandBuffers()
{
	VkResult result;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 2;
	
	commandBuffers.resize(2);
	result = vkAllocateCommandBuffers(vkInfo.device, &commandBufferAllocateInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create command buffers!", "Error", MB_OK);
		exit(-1);
	}

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	inFlightFences.resize(2);
	for (int i = 0; i < 2; ++i) {
		result = vkCreateFence(vkInfo.device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to create fence!", "Error", MB_OK);
			exit(-1);
		}
	}
}

VkSurfaceFormatKHR VulkanRender::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	VkSurfaceFormatKHR result;

	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		result.format = VK_FORMAT_B8G8R8A8_UNORM;
		result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		return result;
	}

	for (int i = 0; i < availableFormats.size(); ++i)
	{
		VkSurfaceFormatKHR availableFormat = availableFormats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			result = availableFormat;
			return result;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanRender::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	const VkPresentModeKHR bestMode = VK_PRESENT_MODE_MAILBOX_KHR;

	for (int i = 0; i < availablePresentModes.size(); ++i)
	{
		if (availablePresentModes[i] == bestMode)
		{
			return bestMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRender::chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& capabilities, int height, int width)
{
	VkExtent2D result;
	if (capabilities.currentExtent.width == -1)
	{
		result.width = height;
		result.height = width;
	}
	else
	{
		result = capabilities.currentExtent;
	}
	return result;
}

void VulkanRender::createSwapChain(int height, int width)
{
	VkResult result;

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
	VkExtent2D extent = chooseSurfaceExtent(vkInfo.surfaceCapabilities, height, width);

	vkInfo.surfaceFormatClass = surfaceFormat;
	presentModeClass = presentMode;
	swapChainExtentClass = extent;

	swapChainExtent = extent;
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.pNext = nullptr;
	swapChainCreateInfo.flags = 0;
	swapChainCreateInfo.surface = vkInfo.surface;
	swapChainCreateInfo.minImageCount = 2;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	std::vector<uint32_t> indices = { 1, 2 };
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	swapChainCreateInfo.queueFamilyIndexCount = 2;
	swapChainCreateInfo.pQueueFamilyIndices = indices.data();

	swapChainCreateInfo.preTransform = vkInfo.surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(vkInfo.device, &swapChainCreateInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create swap chain!", "Error", MB_OK);
		exit(-1);
	}

	uint32_t numImages = 0;
	std::vector<VkImage> swapChainImages;

	result = vkGetSwapchainImagesKHR(vkInfo.device, swapChain, &numImages, nullptr);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to get swap chain images!", "Error", MB_OK);
		exit(-1);
	}
	swapChainImages.resize(numImages);
	result = vkGetSwapchainImagesKHR(vkInfo.device, swapChain, &numImages, swapChainImages.data());
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to get swap chain images!", "Error", MB_OK);
		exit(-1);
	}
	swapChainImageViews.resize(2);
	for (uint32_t i = 0; i < 2; ++i) {
		VkImageViewCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		imageCreateInfo.image = swapChainImages[i];
		imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageCreateInfo.format = surfaceFormat.format;
		imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		imageCreateInfo.subresourceRange.baseMipLevel = 0;
		imageCreateInfo.subresourceRange.levelCount = 1;

		imageCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageCreateInfo.subresourceRange.layerCount = 1;
		imageCreateInfo.flags = 0;

		result = vkCreateImageView(vkInfo.device, &imageCreateInfo, nullptr, &swapChainImageViews[i]);
	}
	OutputDebugString("VULKAN DONE!");
}

VkFormat VulkanRender::ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	for (int i = 0; i < formats.size(); ++i)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(gpuInfo.physicalDevice, formats[i], &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return formats[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return formats[i];
		}
	}
	MessageBox(NULL, "Failed to find supported format!", "Error", MB_OK);
	return VK_FORMAT_UNDEFINED;
}


void VulkanRender::createRenderTargets()
{
	{
		std::vector<VkFormat> formats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT };

		depthFormat = ChooseSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	createImage(swapChainExtentClass.width, swapChainExtentClass.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageViews(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRender::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachments;
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vkInfo.surfaceFormatClass.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments.push_back(colorAttachment);

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachments.push_back(depthAttachment);

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 0;

	VkResult result = vkCreateRenderPass(vkInfo.device, &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create render pass!", "Error", MB_OK);
		exit(-1);
	}

}

void VulkanRender::createFrameBuffers()
{
	VkImageView attachments[2];
	attachments[1] = depthImageView;
	
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = attachments;
	framebufferCreateInfo.width = swapChainExtent.width;
	framebufferCreateInfo.height = swapChainExtent.height;
	framebufferCreateInfo.layers = 1;

	swapChainFramebuffers.resize(2);
	for (int i = 0; i < 2; ++i)
	{
		attachments[0] = swapChainImageViews[i];
		VkResult result = vkCreateFramebuffer(vkInfo.device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			
			MessageBox(NULL, "Failed to create framebuffer!", "Error", MB_OK);
			exit(-1);
		}
	}
}

VkImage VulkanRender::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.flags = 0;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.requiredFlags = properties;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	
	allocation.resize(allocation.size() + 1);

	VkResult result = vmaCreateImage(vkInfo.allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation.at(allocation.size() - 1).allocation, nullptr);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create image!", "Error", MB_OK);
		exit(-1);
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkInfo.device, image, &memoryRequirements);
	/*
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryResources(memoryRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(vkInfo.device, &memoryAllocateInfo, nullptr, &imageMemory);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to allocate image memory!", "Error", MB_OK);
		exit(-1);
	}

	vkBindImageMemory(vkInfo.device, image, imageMemory, 0);*/
	images.push_back(image);
	return image;
}

VkImageView VulkanRender::createImageViews(VkImage image, VkFormat format, VkImageAspectFlags flags)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.subresourceRange.aspectMask = flags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult result = vkCreateImageView(vkInfo.device, &imageViewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create image view!", "Error", MB_OK);
		exit(-1);
	}

	return imageView;
}

uint32_t VulkanRender::findMemoryResources(uint32_t memoryRequirements, VkMemoryPropertyFlags properties)
{
	VkResult result;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gpuInfo.physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	MessageBox(NULL, "Failed to find suitable memory type!", "Error", MB_OK);
}

void VulkanRender::LoadShader(const char* path) {

	std::function<void(std::pair<const void*, int64_t>)> shaderFunc;

	shaderFunc = [this](std::pair<const void*, int64_t> filePair) {
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = filePair.second;
		shaderModuleCreateInfo.pCode = (uint32_t*)filePair.first;

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule(vkInfo.device, &shaderModuleCreateInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			MessageBox(NULL, "Failed to create shader module!", "Error", MB_OK);
			exit(-1);
		}

		shaderModules.push_back(shaderModule);
		return;
	};
	globalManager->getResourceManager()->loadFile(path, shaderFunc);
}

void VulkanRender::LoadVertexData(const char* path)
{
	std::function<void(std::pair<const void*, int64_t>) > vertexFunc;

	AllocatedBuffer newAlloc;
	newAlloc.vertices = globalManager->getResourceManager()->loadObj(path);

	auto vertexData = newAlloc.vertices.get()->size();

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(VertexStruct) * vertexData;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.flags = 0;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;



	VkResult result = vmaCreateBuffer(vkInfo.allocator, &bufferCreateInfo, &allocationCreateInfo, &newAlloc.buffer, &newAlloc.allocation, nullptr);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to create vertex buffer!", "Error", MB_OK);
		exit(-1);
	}

	void* data;
	vmaMapMemory(vkInfo.allocator, newAlloc.allocation, &data);
	memcpy(data, newAlloc.vertices.get(), vertexData * sizeof(VertexStruct));
	vmaUnmapMemory(vkInfo.allocator, newAlloc.allocation);


	allocation.push_back(newAlloc);
	return;
}

void VulkanRender::buildPipeline(std::string name)
{
	pipelineBuilder.addShaderModule(shaderModules.at(0), VERTEX_SHADER);
	pipelineBuilder.addShaderModule(shaderModules.at(1), FRAGMENT_SHADER);
	VkShaderStageFlagBits stages[] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
	pipelineBuilder.setShaderStages(stages);

	pipelineBuilder.setVertexInputInfo();
	pipelineBuilder.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	pipelineBuilder.buildViewport(swapChainExtent.width, swapChainExtent.height);
	pipelineBuilder.setViewport();

	pipelineBuilder.setRasterizer(VK_POLYGON_MODE_FILL);

	pipelineBuilder.setMultisampling();
	pipelineBuilder.setColorBlendAttachment();

	pipelineBuilder.setPipelineLayout();

	pipelines.push_back(pipelineBuilder.buildPipeline(vkInfo.device, renderPass));
	pipelines.at(pipelines.size() - 1).name = name;
}

void VulkanRender::processShader(std::vector<char>data)
{


}

void VulkanRender::preRender(VkCommandBuffer &cmd, uint32_t &imageIndex)
{
	VkResult result;
	result = vkWaitForFences(vkInfo.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to wait for fence", "Error", MB_OK);
	}

	result = vkResetFences(vkInfo.device, 1, &inFlightFences[currentFrame]);
	
	result = vkAcquireNextImageKHR(vkInfo.device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to acquire swap chain image", "Error", MB_OK);
	}

	result = vkResetCommandBuffer(commandBuffers[imageIndex], 0);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to reset command buffer", "Error", MB_OK);
	}

	cmd = commandBuffers[imageIndex];
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = NULL;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBeginInfo.pInheritanceInfo = NULL;
	
	result = vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	VkClearValue clearValues[2];
	clearValues[0].color = {0.0f, 0.0f, 1.0f, 1.0f};
	clearValues[1].depthStencil = { 1.0f, 0 };
	//clearValues.depthStencil = { 1.0f, 0 };
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRender::postRender(VkCommandBuffer &cmd, uint32_t &imageIndex)
{
	VkResult result;
	vkCmdEndRenderPass(cmd);

	result = vkEndCommandBuffer(cmd);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to end command buffer", "Error", MB_OK);
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.pWaitDstStageMask = &waitStage;

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;

	presentInfo.pSwapchains = &swapChain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(graphicsQueue, &presentInfo);

	currentFrame += 1;
	if (currentFrame > 1)
	{
		currentFrame = 0;
	}
}

bool VulkanRender::allocateMemory(VkDeviceMemory& memory, uint32_t memoryRequirements, uint32_t memoryTypeIndex)
{
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkResult result = vkAllocateMemory(vkInfo.device, &memoryAllocateInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		MessageBox(NULL, "Failed to allocate memory!", "Error", MB_OK);
		return false;
	}

	return true;
}


void VulkanRender::End()
{

}