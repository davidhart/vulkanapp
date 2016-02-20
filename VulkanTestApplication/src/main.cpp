#include <iostream>
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

	VkPhysicalDevice devices[10];
	uint32_t deviceCount;

	result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to enumerate devices" << std::endl;
		return 1;
	}

	std::cout << "Device Count:" << deviceCount << std::endl;

	for (int i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		
		std::cout << "============================================================" << std::endl;
		std::cout << "Api Version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion) 
				<< "." << VK_VERSION_MINOR(deviceProperties.apiVersion) 
				<< "." << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
		std::cout << "Device ID: " <<  deviceProperties.deviceID << std::endl;
		std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
		std::cout << "Device Type: " << deviceProperties.deviceType << std::endl;
		std::cout << "Driver Version: " << VK_VERSION_MAJOR(deviceProperties.driverVersion)
			<< "." << VK_VERSION_MINOR(deviceProperties.driverVersion)
			<< "." << VK_VERSION_PATCH(deviceProperties.driverVersion) << std::endl;
		std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
		std::cout << "VendorID: " << deviceProperties.vendorID << std::endl;
	}

	vkDestroyInstance(instance, NULL);
	return 0;
}
