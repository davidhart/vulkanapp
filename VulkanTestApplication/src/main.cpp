#include <iostream>
#include <vector>

#include "render_window.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
	std::cout << pLayerPrefix << ": " << pMsg << std::endl;
	return 1;
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

	// Enumerate queue family properties
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	std::cout << "Device Queue Family Properties:" << std::endl;
	for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i)
	{
		VkQueueFamilyProperties& props = queueFamilyProperties[i];
		std::cout << "============================================================" << std::endl;
		std::cout << "minImageTransferGranularity- depth:" << props.minImageTransferGranularity.depth
			<< " height:" << props.minImageTransferGranularity.height
			<< " width:" << props.minImageTransferGranularity.width
			<< std::endl;
		std::cout << "queueCount: " << props.queueCount << std::endl;
		std::cout << "queueFlags: " << props.queueFlags << std::endl;
		std::cout << "timestampValidBits: " << props.timestampValidBits << std::endl;
	}


	// For now use a single queue (first available)
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = NULL;
	deviceQueueCreateInfo.flags = 0;
	// First queue from family list
	deviceQueueCreateInfo.queueFamilyIndex = 0;
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

	VkBool32 physicalDeviceSurfaceSupport;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &physicalDeviceSurfaceSupport);

	if (physicalDeviceSurfaceSupport == false)
	{
		std::cout << "Surface not supported for physical device queue 0" << std::endl;
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
	const uint32_t queueFamilyIndices[] = { 0 };
	swapchainCreateInfo.queueFamilyIndexCount = 1;
	swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = false;
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
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0;

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

		for (int i = 0; i < swapchainImages.size(); ++i)
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

	while (renderWindow.IsOpen())
	{
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
		result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompleteSemaphore, NULL, &currentSwapImage);

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
		beginFrameBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		beginFrameBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		beginFrameBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		beginFrameBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		beginFrameBarrier.srcQueueFamilyIndex = 0;
		beginFrameBarrier.dstQueueFamilyIndex = 0;
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
		renderPassBeginInfo.renderArea.extent = surfaceCapabilities.currentExtent;
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

		vkCmdEndRenderPass(commandBuffer);

		VkImageMemoryBarrier endOfFrameBarrier;
		endOfFrameBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		endOfFrameBarrier.pNext = NULL;
		endOfFrameBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		endOfFrameBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		endOfFrameBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		endOfFrameBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		endOfFrameBarrier.srcQueueFamilyIndex = 0;
		endOfFrameBarrier.dstQueueFamilyIndex = 0;
		endOfFrameBarrier.image = swapchainImages[currentSwapImage];
		endOfFrameBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &endOfFrameBarrier);

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't end command buffer" << std::endl;
			return 1;
		}

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = NULL;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
		submitInfo.pWaitDstStageMask = NULL;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = NULL;
		result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't submit command buffer" << std::endl;
		}

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &presentCompleteSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &currentSwapImage;
		presentInfo.pResults = &result;
		result = vkQueuePresentKHR(queue, &presentInfo);

		if (result != VK_SUCCESS)
		{
			std::cout << "Couldn't present buffer" << std::endl;
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
