#include <iostream>
#include <vector>
#include <fstream>

#include "render_window.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
	std::cout << pLayerPrefix << ": " << pMsg << std::endl;
	return false;
}

bool memory_type_from_properties(const VkMemoryType* memoryTypes, uint32_t typeBits, VkFlags requirementsMask, uint32_t* typeIndex)
{
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		if (typeBits & 1 && (memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
		{
			*typeIndex = i;
			return true;
		}
		typeBits >>= 1;
	}

	return false;
}

bool CreateDeviceMemory(VkDevice device, const VkMemoryType* memoryTypes, const VkMemoryRequirements* memoryRequirements, VkFlags requirementsMask, size_t dataSize, VkDeviceMemory* memory)
{
	VkMemoryAllocateInfo bufferAllocateInfo;
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.pNext = NULL;
	bufferAllocateInfo.allocationSize = dataSize;
	bufferAllocateInfo.memoryTypeIndex = 0;

	bool validMemoryType = memory_type_from_properties(memoryTypes, memoryRequirements->memoryTypeBits, requirementsMask, &bufferAllocateInfo.memoryTypeIndex);

	if (validMemoryType == false)
	{
		return false;
	}

	VkResult result = vkAllocateMemory(device, &bufferAllocateInfo, NULL, memory);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool SetDeviceMemory(VkDevice device, VkDeviceMemory memory, void* data, size_t dataSize)
{
	void* mappedMem;
	VkResult result = vkMapMemory(device, memory, 0, dataSize, 0, &mappedMem);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	memcpy(mappedMem, data, dataSize);

	vkUnmapMemory(device, memory);

	return true;
}

bool CreateBuffer(VkDevice device, VkBufferUsageFlags usageFlags, const VkMemoryType* memoryTypes, void* data, size_t dataSize, VkBuffer* buffer, VkDeviceMemory* memory)
{
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = dataSize;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = NULL;

	VkResult result = vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

	if (CreateDeviceMemory(device, memoryTypes, &memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, dataSize, memory) == false)
	{
		return false;
	}

	if (data != NULL)
	{
		if (SetDeviceMemory(device, *memory, data, dataSize) == false)
		{
			return false;
		}
	}

	result = vkBindBufferMemory(device, *buffer, *memory, 0);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool CreateImage2D(VkDevice device, uint32_t width, uint32_t height, VkFormat format, const VkMemoryType* memoryTypes, void* data, size_t dataSize, VkImage* image, VkDeviceMemory* memory)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = NULL;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR; 	// TODO:  THIS MIGHT NOT BE SUPPORTED, may need to use staging texture!
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = NULL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkResult result = vkCreateImage(device, &imageCreateInfo, NULL, image);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, *image, &memoryRequirements);

	if (CreateDeviceMemory(device, memoryTypes, &memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, dataSize, memory) == false)
	{
		return false;
	}

	if (SetDeviceMemory(device, *memory, data, dataSize) == false)
	{
		return false;
	}

	result = vkBindImageMemory(device, *image, *memory, 0);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

int main(char** argv, int argc)
{
	VkApplicationInfo appInfo;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "VulkanTestApplication";
	appInfo.pEngineName = "VulkanTestApplication";
	appInfo.pNext = NULL;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	std::vector<const char*> enabledExtensions = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME
	};

	std::vector<const char*> enabledLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	VkInstanceCreateInfo createInfo;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = NULL;
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledLayerNames = enabledLayers.data();
	createInfo.enabledLayerCount = enabledLayers.size();
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkInstance instance;

	VkResult result;

	result = vkCreateInstance(&createInfo, NULL, &instance);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create instance" << std::endl;
		return 1;
	}

	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pNext = NULL;
	debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debugReportCallbackCreateInfo.pfnCallback = debug_callback;
	debugReportCallbackCreateInfo.pUserData = NULL;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	VkDebugReportCallbackEXT debugCallback;
	result = vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, NULL, &debugCallback);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to install debug report callback" << std::endl;
	}

	// Get number of physical devices
	uint32_t deviceCount = 0;
	result = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to enumerate devices" << std::endl;
		return 1;
	}

	if (deviceCount == 0)
	{
		std::cout << "No devices found" << std::endl;
		return 1;
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

	result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to enumerate devices" << std::endl;
	}

	std::cout << "Device Count:" << deviceCount << std::endl;

	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

		std::cout << "============================================================" << std::endl;
		std::cout << "Api Version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion)
			<< "." << VK_VERSION_MINOR(deviceProperties.apiVersion)
			<< "." << VK_VERSION_PATCH(deviceProperties.apiVersion)
			<< std::endl;
		std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
		std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
		std::cout << "Device Type: " << deviceProperties.deviceType << std::endl;
		std::cout << "Driver Version: " << VK_VERSION_MAJOR(deviceProperties.driverVersion)
			<< "." << VK_VERSION_MINOR(deviceProperties.driverVersion)
			<< "." << VK_VERSION_PATCH(deviceProperties.driverVersion)
			<< std::endl;
		std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
		std::cout << "VendorID: " << deviceProperties.vendorID << std::endl;
	}

	// For now pick the first available physical device..
	VkPhysicalDevice physicalDevice = physicalDevices[0];


	RenderWindow renderWindow;
	renderWindow.Create();
	renderWindow.Show();

	VkSurfaceKHR surface;
	// Begin Windows specific
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = renderWindow.GetNativeHandle();
	result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create win32 surface" << std::endl;
		return 1;
	}
	// End Windows Specific

	// Enumerate queue family properties
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	uint32_t graphicsQueueIndex = UINT32_MAX;

	for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i)
	{
		VkQueueFamilyProperties& props = queueFamilyProperties[i];
		
		VkBool32 surfaceSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupport);

		if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT && surfaceSupport)
		{
			graphicsQueueIndex = i;
			break;
		}
	}

	if (graphicsQueueIndex == UINT32_MAX)
	{
		std::cout << "No compatible device queue found" << std::endl;
		return 1;
	}


	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	// For now use a single queue (first available)
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = NULL;
	deviceQueueCreateInfo.flags = 0;
	// First queue from family list
	deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	// Single queue
	const float queuePriorities[] = { 1.0f }; // normalized floats, 1.0 is highest priority
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;
	deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = enabledLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkDevice device;
	result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create logical device" << std::endl;
		return 1;
	}

	uint32_t formatCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to get device surface formats" << std::endl;
		return 1;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to get device surface formats" << std::endl;
		return 1;
	}

	if (formatCount < 0)
	{
		std::cout << "No surface formats" << std::endl;
		return 1;
	}

	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;

	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		colorFormat = surfaceFormats[0].format;
	}

	colorSpace = surfaceFormats[0].colorSpace;

	std::cout << "colorFormat: " << colorFormat << std::endl;
	std::cout << "colorSpace: " << colorSpace << std::endl;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

	// Create swapchain
	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = NULL;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 2;
	swapchainCreateInfo.imageFormat = colorFormat;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent; 	// Window width / height
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = NULL;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.oldSwapchain = NULL;

	VkSwapchainKHR swapchain;
	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);

	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create swapchain" << std::endl;
		return 1;
	}

	// Get swapchain images
	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't get swapchain image count" << std::endl;
		return 1;
	}

	std::vector<VkImage> swapchainImages(swapchainImageCount);
	result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't get swapchain images" << std::endl;
		return 1;
	}

	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = colorFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference;
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.pDepthStencilAttachment = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = NULL;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = NULL;

	VkRenderPass renderPass;
	result = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);

	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create vk renderpass" << std::endl;
		return 1;
	}

	std::vector<VkImageView> backbufferViews(swapchainImageCount);
	std::vector<VkFramebuffer> framebuffers(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo backbufferViewCreateInfo;
		backbufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		backbufferViewCreateInfo.pNext = NULL;
		backbufferViewCreateInfo.flags = 0;
		backbufferViewCreateInfo.image = swapchainImages[i];
		backbufferViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		backbufferViewCreateInfo.format = colorFormat;
		backbufferViewCreateInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		backbufferViewCreateInfo.subresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};

		result = vkCreateImageView(device, &backbufferViewCreateInfo, NULL, &backbufferViews[i]);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create backbuffer image view " << i << std::endl;
			return 1;
		}

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = NULL;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		VkImageView backbufferView = backbufferViews[i];
		framebufferCreateInfo.pAttachments = &backbufferView;
		framebufferCreateInfo.width = surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height = surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		result = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &framebuffer);

		framebuffers[i] = framebuffer;

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create framebuffer" << std::endl;
			return 1;
		}
	}

	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = NULL;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;

	VkCommandPool commandPool;
	result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);

	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create command pool" << std::endl;
		return 1;
	}

	// Initialise framebuffers
	{
		VkCommandBufferAllocateInfo initCommandBufferAllocateInfo;
		initCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		initCommandBufferAllocateInfo.pNext = NULL;
		initCommandBufferAllocateInfo.commandPool = commandPool;
		initCommandBufferAllocateInfo.commandBufferCount = 1;
		initCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandBuffer initCommandBuffer;
		result = vkAllocateCommandBuffers(device, &initCommandBufferAllocateInfo, &initCommandBuffer);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't allocate command buffer" << std::endl;
			return 1;
		}

		VkCommandBufferInheritanceInfo initCommandBufferInheritanceInfo;
		initCommandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		initCommandBufferInheritanceInfo.pNext = NULL;
		initCommandBufferInheritanceInfo.renderPass = VK_NULL_HANDLE;
		initCommandBufferInheritanceInfo.subpass = 0;
		initCommandBufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
		initCommandBufferInheritanceInfo.occlusionQueryEnable = VK_FALSE;
		initCommandBufferInheritanceInfo.queryFlags = 0;
		initCommandBufferInheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo initCommandBufferBeginInfo;
		initCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		initCommandBufferBeginInfo.pNext = NULL;
		initCommandBufferBeginInfo.flags = 0;
		initCommandBufferBeginInfo.pInheritanceInfo = &initCommandBufferInheritanceInfo;

		result = vkBeginCommandBuffer(initCommandBuffer, &initCommandBufferBeginInfo);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't begin command buffer" << std::endl;
			return 1;
		}

		for (size_t i = 0; i < swapchainImages.size(); ++i)
		{
			VkImageMemoryBarrier initBackbufferBarrier;
			initBackbufferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			initBackbufferBarrier.pNext = NULL;
			initBackbufferBarrier.srcAccessMask = 0;
			initBackbufferBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			initBackbufferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			initBackbufferBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			initBackbufferBarrier.srcQueueFamilyIndex = 0;
			initBackbufferBarrier.dstQueueFamilyIndex = 0;
			initBackbufferBarrier.image = swapchainImages[i];
			initBackbufferBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(initCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &initBackbufferBarrier);
		}

		result = vkEndCommandBuffer(initCommandBuffer);
		
		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't end command buffer" << std::endl;
			return 1;
		}

		VkSubmitInfo initSubmitInfo;
		initSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		initSubmitInfo.pNext = NULL;
		initSubmitInfo.waitSemaphoreCount = 0;
		initSubmitInfo.pWaitSemaphores = NULL;
		initSubmitInfo.pWaitDstStageMask = NULL;
		initSubmitInfo.commandBufferCount = 1;
		initSubmitInfo.pCommandBuffers = &initCommandBuffer;
		initSubmitInfo.signalSemaphoreCount = 0;
		initSubmitInfo.pSignalSemaphores = NULL;
		result = vkQueueSubmit(queue, 1, &initSubmitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't submit command buffer" << std::endl;
			return 1;
		}

		result = vkQueueWaitIdle(queue);
		if (result != VK_SUCCESS)
		{
			std::cout << "Wait idle failed" << std::endl;
			return 1;
		}

		vkFreeCommandBuffers(device, commandPool, 1, &initCommandBuffer);
	}

	// Initialise drawable
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = NULL;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

	VkDescriptorSetLayout descriptorSetLayout;
	result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create decriptor set" << std::endl;
		return 1;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = NULL;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = 0;

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create pipeline layout" << std::endl;
		return 1;
	}

	VkShaderModule vertModule;
	{
		std::ifstream vertfile("tri.vert.spv", std::ios::binary);
		if (vertfile.is_open() == false)
		{
			std::cout << "Couldn't read tri.vert.spv" << std::endl;
			return 1;
		}

		std::vector<char> contents;
		contents.reserve(10000);
		contents.assign(std::istreambuf_iterator<char>(vertfile), std::istreambuf_iterator<char>());

		VkShaderModuleCreateInfo vertModuleCreateInfo;
		vertModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertModuleCreateInfo.pNext = NULL;
		vertModuleCreateInfo.flags = 0;
		vertModuleCreateInfo.codeSize = contents.size();
		vertModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(contents.data());

		result = vkCreateShaderModule(device, &vertModuleCreateInfo, NULL, &vertModule);
		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create vert shader module" << std::endl;
			return 1;
		}
	}

	VkShaderModule fragModule;
	{
		std::ifstream fragfile("tri.frag.spv", std::ios::binary);
		if (fragfile.is_open() == false)
		{
			std::cout << "Couldn't read tri.frag.spv" << std::endl;
			return 1;
		}

		std::vector<char> contents;
		contents.reserve(10000);
		contents.assign(std::istreambuf_iterator<char>(fragfile), std::istreambuf_iterator<char>());

		VkShaderModuleCreateInfo vertModuleCreateInfo;
		vertModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertModuleCreateInfo.pNext = NULL;
		vertModuleCreateInfo.flags = 0;
		vertModuleCreateInfo.codeSize = contents.size();
		vertModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(contents.data());

		result = vkCreateShaderModule(device, &vertModuleCreateInfo, NULL, &fragModule);
		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create vert shader module" << std::endl;
			return 1;
		}
	}

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCreateInfo.pNext = NULL;
	pipelineCacheCreateInfo.flags = 0;
	pipelineCacheCreateInfo.initialDataSize = 0;
	pipelineCacheCreateInfo.pInitialData = NULL;

	VkPipelineCache pipelineCache;
	result = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL, &pipelineCache);
	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create pipeline cache" << std::endl;
		return 1;
	}

	VkPipeline pipeline;
	{
		VkPipelineShaderStageCreateInfo stages[2];
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].pNext = NULL;
		stages[0].flags = 0;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = vertModule;
		stages[0].pName = "main";
		stages[0].pSpecializationInfo = NULL;

		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].pNext = NULL;
		stages[1].flags = 0;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = fragModule;
		stages[1].pName = "main";
		stages[1].pSpecializationInfo = NULL;

		VkVertexInputBindingDescription vertexBindingDescription;
		vertexBindingDescription.binding = 0;
		vertexBindingDescription.stride = sizeof(float) * 6;
		vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertexAttributes[2];
		vertexAttributes[0].binding = 0;
		vertexAttributes[0].location = 0;
		vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributes[0].offset = 0;

		vertexAttributes[1].binding = 0;
		vertexAttributes[1].location = 1;
		vertexAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributes[1].offset = sizeof(float) * 3;

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.pNext = NULL;
		vertexInputCreateInfo.flags = 0;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
		vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributes;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.pNext = NULL;
		inputAssemblyCreateInfo.flags = 0;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
		
		VkPipelineViewportStateCreateInfo viewportCreateInfo;
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.pNext = NULL;
		viewportCreateInfo.flags = 0;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.pViewports = NULL;
		viewportCreateInfo.scissorCount = 1;
		viewportCreateInfo.pScissors = NULL;

		VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
		rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationCreateInfo.pNext = NULL;
		rasterizationCreateInfo.flags = 0;
		rasterizationCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationCreateInfo.depthBiasConstantFactor = 0.f;
		rasterizationCreateInfo.depthBiasClamp = 0.f;
		rasterizationCreateInfo.depthBiasSlopeFactor = 0.f;
		rasterizationCreateInfo.lineWidth = 0.f;

		VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
		multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleCreateInfo.pNext = NULL;
		multisampleCreateInfo.flags = 0;
		multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleCreateInfo.minSampleShading = 0.f;
		multisampleCreateInfo.pSampleMask = NULL;
		multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
		depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.pNext = NULL;
		depthStencilCreateInfo.flags = 0;
		depthStencilCreateInfo.depthTestEnable = VK_FALSE;
		depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
		depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
		depthStencilCreateInfo.front = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 };
		depthStencilCreateInfo.back = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 };
		depthStencilCreateInfo.minDepthBounds = 0.f;
		depthStencilCreateInfo.maxDepthBounds = 1.f;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
		colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendCreateInfo.pNext = NULL;
		colorBlendCreateInfo.flags = 0;
		colorBlendCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendCreateInfo.logicOp = VK_LOGIC_OP_CLEAR;
		colorBlendCreateInfo.attachmentCount = 1;
		colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendCreateInfo.blendConstants[0] = 1.f;
		colorBlendCreateInfo.blendConstants[1] = 1.f;
		colorBlendCreateInfo.blendConstants[2] = 1.f;
		colorBlendCreateInfo.blendConstants[3] = 1.f;


		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicCreateInfo;
		dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicCreateInfo.pNext = NULL;
		dynamicCreateInfo.flags = 0;
		dynamicCreateInfo.dynamicStateCount = 2;
		dynamicCreateInfo.pDynamicStates = dynamicStates;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = NULL;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = stages;
		pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineCreateInfo.pTessellationState = NULL;
		pipelineCreateInfo.pViewportState = &viewportCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
		pipelineCreateInfo.pDynamicState = &dynamicCreateInfo;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = 0;

		result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, NULL, &pipeline);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create graphics pipeline" << std::endl;
			return 1;
		}
	}

	VkBuffer vertBuffer;
	VkDeviceMemory vertDeviceMemory;

	const float buffer[3][6] = {
		{ -1.0f, -1.0f,  0.25f,     1.0f, 0.0f, 0.0f },
		{ 1.0f, -1.0f,  0.25f,      0.0f, 1.0f, 0.0f },
		{ 0.0f,  1.0f,  1.0f,       0.0f, 0.0f, 1.0f },
	};
	size_t bufferSize = sizeof(buffer);

	bool vertBufferCreated = CreateBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryProperties.memoryTypes, (void*)buffer, bufferSize, &vertBuffer, &vertDeviceMemory);
	
	if (vertBufferCreated == false)
	{
		std::cout << "Couldn't create vertex buffer" << std::endl;
		return 1;
	}

	VkImage texture;
	VkDeviceMemory textureMemory;

	// Init texture
	{
		const size_t width = 8;
		const size_t height = 8;
		const size_t numPixels = width * height;
		const size_t textureSize = numPixels * 4;
		unsigned char data[textureSize];

		const char* textureMap =
			"........"
			"..#..#.."
			"..#..#.."
			"..#..#.."
			"........"
			".#....#."
			"..####.."
			"........";

		unsigned char bgCol[4] = { 255, 255, 255, 255 };
		unsigned char fgCol[4] = { 0,   0,   0,   255 };
		int bufferIndex = 0;

		for (int i = 0; i < numPixels; i++)
		{
			unsigned char* col = textureMap[i] == '#' ? fgCol : bgCol;
			data[bufferIndex++] = col[0];
			data[bufferIndex++] = col[1];
			data[bufferIndex++] = col[2];
			data[bufferIndex++] = col[3];
		}

		bool created = CreateImage2D(device, width, height, VK_FORMAT_R8G8B8A8_UNORM, memoryProperties.memoryTypes, (void*)data, textureSize, &texture, &textureMemory);

		if (created == false)
		{
			std::cout << "Couldn't create texture" << std::endl;
			return 1;
		}
	}

	VkSampler textureSampler;
	VkImageView textureImageView;

	{
		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = NULL;
		samplerCreateInfo.flags = 0;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(device, &samplerCreateInfo, NULL, &textureSampler);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create texture sampler" << std::endl;
			return 1;
		}

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = NULL;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = texture;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		result = vkCreateImageView(device, &imageViewCreateInfo, NULL, &textureImageView);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create texture image view" << std::endl;
			return 1;
		}
	}

	VkBuffer uniformBuffer;
	VkDeviceMemory uniformDeviceMemory;
	size_t uniformSize = sizeof(float)*16;
	bool uniformBufferCreated = CreateBuffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryProperties.memoryTypes, NULL, uniformSize, &uniformBuffer, &uniformDeviceMemory);

	if (uniformBufferCreated == false)
	{
		std::cout << "Couldn't create uniform buffer" << std::endl;
		return 1;
	}

	VkDescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.descriptorCount = 1;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

	VkDescriptorPool descriptorPool;
	result = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool);

	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't create descriptor pool" << std::endl;
	}

	VkDescriptorSet descriptorSet;
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool; 
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);

	if (result != VK_SUCCESS)
	{
		std::cout << "Couldn't allocate descriptor set" << std::endl;
		return 1;
	}

	VkDescriptorBufferInfo uniformBufferInfo;
	uniformBufferInfo.buffer = uniformBuffer;
	uniformBufferInfo.offset = 0;
	uniformBufferInfo.range = uniformSize;

	VkWriteDescriptorSet uniformWrite;
	uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWrite.pNext = NULL;
	uniformWrite.dstSet = descriptorSet;
	uniformWrite.dstBinding = 0;
	uniformWrite.dstArrayElement = 0;
	uniformWrite.descriptorCount = 1;
	uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWrite.pImageInfo = NULL;
	uniformWrite.pBufferInfo = &uniformBufferInfo;
	uniformWrite.pTexelBufferView = NULL;
	vkUpdateDescriptorSets(device, 1, &uniformWrite, 0, NULL);


	float t = 0.0f;

	while (renderWindow.IsOpen())
	{
		t += 0.0001f;

		void* mappedUniform;
		result = vkMapMemory(device, uniformDeviceMemory, 0, uniformSize, 0, &mappedUniform);

		float uniformData[16] = { cos(t), sin(t), 0.0f, 0.0f,
								  -sin(t), cos(t), 0.0f, 0.0f,
								  0.0f, 0.0f, 1.0f, 0.0f,
								  0.0f, 0.0f, 0.0f, 1.0f };

		if (result != VK_SUCCESS)
		{
			std::cout << "Unable to map uniform memory" << std::endl;
			return 1;
		}

		memcpy(mappedUniform, uniformData, uniformSize);
		vkUnmapMemory(device, uniformDeviceMemory);

		VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
		presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		presentCompleteSemaphoreCreateInfo.pNext = NULL;
		presentCompleteSemaphoreCreateInfo.flags = 0;

		VkSemaphore presentCompleteSemaphore;
		result = vkCreateSemaphore(device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't create present complete semaphore" << std::endl;
			return 1;
		}

		uint32_t currentSwapImage;
		result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentSwapImage);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't aquire swapchain image" << std::endl;

			// if out of date respond to window resize?
			return 1;
		}
		
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = NULL;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandBuffer commandBuffer;
		result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't allocate command buffer" << std::endl;
			return 1;
		}

		VkCommandBufferInheritanceInfo commandBufferInheritanceInfo;
		commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		commandBufferInheritanceInfo.pNext = NULL;
		commandBufferInheritanceInfo.renderPass = VK_NULL_HANDLE;
		commandBufferInheritanceInfo.subpass = 0;
		commandBufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
		commandBufferInheritanceInfo.occlusionQueryEnable = VK_FALSE;
		commandBufferInheritanceInfo.queryFlags = 0;
		commandBufferInheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = NULL;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;
		result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't begin command buffer" << std::endl;
			return 1;
		}
		
		VkImageMemoryBarrier beginFrameBarrier;
		beginFrameBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		beginFrameBarrier.pNext = NULL;
		beginFrameBarrier.srcAccessMask = 0;
		beginFrameBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		beginFrameBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		beginFrameBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		beginFrameBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beginFrameBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beginFrameBarrier.image = swapchainImages[currentSwapImage];
		beginFrameBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &beginFrameBarrier);

		VkClearValue clearValue;
		clearValue.color.float32[0] = (float)rand() / (float)RAND_MAX;
		clearValue.color.float32[1] = (float)rand() / (float)RAND_MAX;
		clearValue.color.float32[2] = (float)rand() / (float)RAND_MAX;
		clearValue.color.float32[3] = 1.0f;

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[currentSwapImage];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width = surfaceCapabilities.currentExtent.width;
		renderPassBeginInfo.renderArea.extent.height = surfaceCapabilities.currentExtent.width;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		for (int i = 1; i < 5; ++i)
		{
			clearValue.color.float32[0] = (float)rand() / (float)RAND_MAX;
			clearValue.color.float32[1] = (float)rand() / (float)RAND_MAX;
			clearValue.color.float32[2] = (float)rand() / (float)RAND_MAX;
			clearValue.color.float32[3] = 1.0f;

			VkClearAttachment clearAttachment;
			clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clearAttachment.clearValue = clearValue;
			clearAttachment.colorAttachment = 0;

			VkClearRect clearRect;
			clearRect.baseArrayLayer = 0;
			clearRect.layerCount = 1;
			clearRect.rect.offset = { i * 20, i * 20 };
			clearRect.rect.extent = { surfaceCapabilities.currentExtent.width - i * 40, surfaceCapabilities.currentExtent.height - i * 40 };
			vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)surfaceCapabilities.currentExtent.width;
		viewport.height = (float)surfaceCapabilities.currentExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.extent.width = surfaceCapabilities.currentExtent.width;
		scissor.extent.height = surfaceCapabilities.currentExtent.height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertBuffer, &offset);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);



		vkCmdEndRenderPass(commandBuffer);

		VkImageMemoryBarrier endOfFrameBarrier;
		endOfFrameBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		endOfFrameBarrier.pNext = NULL;
		endOfFrameBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		endOfFrameBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		endOfFrameBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		endOfFrameBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		endOfFrameBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		endOfFrameBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		endOfFrameBarrier.image = swapchainImages[currentSwapImage];
		endOfFrameBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &endOfFrameBarrier);

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't end command buffer" << std::endl;
			return 1;
		}

		VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = NULL;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = NULL;
		result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't submit command buffer" << std::endl;
		}

		VkResult queuePresentResult = VK_SUCCESS;

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &currentSwapImage;
		presentInfo.pResults = &queuePresentResult;
		
		result = vkQueuePresentKHR(queue, &presentInfo);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't present buffer" << std::endl;
			return 1;
		}

		if (queuePresentResult != VK_SUCCESS)
		{
			std::cout << "Queue present failed" << std::endl;
			return 1;
		}

		result = vkQueueWaitIdle(queue);

		if (result != VK_SUCCESS)
		{
			std::cout << "Wait Idle failed" << std::endl;
			return 1;
		}

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

		vkDestroySemaphore(device, presentCompleteSemaphore, NULL);

		renderWindow.DispatchEvents();
	}

	vkDestroyInstance(instance, NULL);
	return 0;
}
