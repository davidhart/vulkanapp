#include <iostream>
#include <vector>

#include <vulkan\vulkan.h>

#define MAX_DEVICES

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

	VkInstanceCreateInfo createInfo;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = NULL;
	createInfo.ppEnabledExtensionNames = NULL;
	createInfo.ppEnabledLayerNames = NULL;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkInstance instance;

	VkResult result;
	
	result = vkCreateInstance(&createInfo, NULL, &instance);
	
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create instance" << std::endl;
		return 1;
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

	for (int i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
		
		std::cout << "============================================================" << std::endl;
		std::cout << "Api Version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion) 
			<< "." << VK_VERSION_MINOR(deviceProperties.apiVersion) 
			<< "." << VK_VERSION_PATCH(deviceProperties.apiVersion) 
			<< std::endl;
		std::cout << "Device ID: " <<  deviceProperties.deviceID << std::endl;
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
	for (int i = 0; i < queueFamilyPropertyCount; ++i)
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

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = NULL;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = NULL;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkDevice device;
	result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create logical device" << std::endl;
		return 1;
	}
	vkDestroyInstance(instance, NULL);
	return 0;
}
