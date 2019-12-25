#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


/*
Steps

1. Make the SDL Window
2. Create An Vulkan instance
	a. Grab the right extensions (sdl and validation layers)
	b. add debug callbacks for validation layers
3. Make a VkSurface using SDL (this is what vulkan points to for our window)
4. Create a logical Device
	a. enumerate physical devices and pick one
	b. from the device, grab the right queue_family for the job (usually w/ present and graphics flag)
	c. create the device
5. Create a swap-chain that points to our VkSurface or a depth buffer
6. Create a command buffer to the logical device
	a. create a command pool(manages memory for command buffers)
		i. make sure the command pool is to the right queue family and device
	b. allocate a command buffer from the pool

*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

bool memType(VkPhysicalDeviceMemoryProperties& props, uint32_t typeBits, VkFlags requirements, uint32_t* typeIndex)
{
	for(uint32_t i = 0; i < props.memoryTypeCount; i++)
	{
		if((typeBits & 1) == 1)
	}
}


std::vector<const char*> validationLayers;

bool checkValidationLayerSupport() {
	
	
	validationLayers.push_back("VK_LAYER_KHRONOS_validation");
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

typedef struct {
	VkImage image;
	VkImageView view;
} swap_chain_buffer;

void derror(const char* err)
{
	perror(err);
	abort();
}

void derror(std::string str)
{
	//str += "\n";
	printf("%s\n", str.c_str());
	abort();
}


int main(/*int argc, char const *argv[]*/)
{
	
	uint32_t wWidth = 1280, wHeight = 720;

	if(!checkValidationLayerSupport()){
		derror("No validationLayers!");
	}

	//create an sdl window
	SDL_Window* window;
	VkSurfaceKHR surface;
	SDL_Init(SDL_INIT_VIDEO);


	window = SDL_CreateWindow("My App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
	if(window == NULL)
		derror(std::string("Could not create window: ") + SDL_GetError());

	//get the right extensions for SDL2
	uint32_t extensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
	std::vector<const char *> extensionNames(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());
	extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	//creating instance--------------------------------------------------------
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Vulkan Test";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Expert Test";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0; //vulkan 1.0 or 1.1?


	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = extensionNames.size();
	inst_info.ppEnabledExtensionNames = extensionNames.data();
	inst_info.enabledLayerCount = validationLayers.size(); //1 validation layer(s)
	inst_info.ppEnabledLayerNames = validationLayers.data();


	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	inst_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

	
	










	VkInstance inst;
	
	VkResult res = vkCreateInstance(&inst_info, nullptr, &inst);
	if(res == VK_ERROR_INCOMPATIBLE_DRIVER)
		derror("Could not find a compatible Vulkan ICD!\n");
	else if(res)
		derror("Unknown error!\n");
	
	if(!SDL_Vulkan_CreateSurface(window, inst, &surface))
		derror(std::string("Could not create window: ") + SDL_GetError());

	VkDebugUtilsMessengerEXT debugMessenger;
	CreateDebugUtilsMessengerEXT(inst, &debugCreateInfo, nullptr, &debugMessenger);


	//end create instance------------------------------------------------------

	//device enumeration-------------------------------------------------------


	uint32_t gpu_count = 1;
	res = vkEnumeratePhysicalDevices(inst, &gpu_count, nullptr);
	assert(gpu_count);
	std::vector<VkPhysicalDevice> gpus;
	gpus.resize(gpu_count);
	res = vkEnumeratePhysicalDevices(inst, &gpu_count, gpus.data());


	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPhysicalDeviceProperties.html
	VkPhysicalDeviceProperties gpuProps[gpus.size()];
	printf("Compatible Physical GPUs:\n");
	for(uint32_t i = 0; i < gpus.size(); i++)
	{
		vkGetPhysicalDeviceProperties(gpus[i], &(gpuProps[i]));
		printf("%d: %s\n", i, gpuProps[i].deviceName);
	}


	//end device enumeration---------------------------------------------------

	//device initialization----------------------------------------------------

	//first we need the right device queue w/ both the graphics and present
	//present just means that it can 'present' to the window

	VkDeviceQueueCreateInfo queue_info = {};
	uint32_t 	queue_family_count = 0, 
				present_queue_family_index = UINT32_MAX, graphics_queue_family_index = UINT32_MAX;
	std::vector<VkQueueFamilyProperties> queue_props;

	//get device queue props
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, nullptr);
	assert(queue_family_count >= 1);
	queue_props.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, queue_props.data());

	VkBool32 *pSupportsPresent = (VkBool32*)malloc(queue_family_count * sizeof(VkBool32));
	for(uint32_t i = 0; i < queue_family_count; i++)
		vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
	

	//search for graphics and present queue in queue array
	
	for(uint32_t i = 0; i < queue_family_count; ++i)
	{
		if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if(graphics_queue_family_index == UINT32_MAX)
				graphics_queue_family_index = i;

			if(pSupportsPresent[i] == VK_TRUE)
			{
				graphics_queue_family_index = i;
				present_queue_family_index = i;
				break;
			}
		}
	}

	if(present_queue_family_index == UINT32_MAX) //todo: this is a shitty way of checking if it's unset
	{
		//if we couldn't find a queue that supports both graphics and present,
		//find a seperate present queue
		for(uint32_t i = 0; i < queue_family_count; ++i)
		{
			if(pSupportsPresent[i] == VK_TRUE)
			{
				present_queue_family_index = i;
				break;
			}
		}
	}

	free(pSupportsPresent);

	if(graphics_queue_family_index == UINT32_MAX 
		|| present_queue_family_index == UINT32_MAX)
		derror("Couldn't find queues for both graphics and present!");


	if(graphics_queue_family_index != present_queue_family_index)
		derror("Graphics index != present index!");




	float queue_priorities[1] = {0.0}; //this is only for dealing w/ multiple queues, so we don't care
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = nullptr;
	queue_info.queueCount = 1; //like i said, you can use multiple queues
	queue_info.pQueuePriorities = queue_priorities; 


	//now we can create a logical device using the queue we found

	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = nullptr;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = deviceExtensions.size();
	device_info.ppEnabledExtensionNames = deviceExtensions.data();
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = nullptr;
	device_info.pEnabledFeatures = nullptr;

	VkDevice device;
	res = vkCreateDevice(gpus[0], &device_info, nullptr, &device);
	assert(res == VK_SUCCESS);


	//end device intialization-------------------------------------------------
	
	//create swap-chain/windowing related crap--------------------------------- 


	//get all of the supported VkFormats;
	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, nullptr);
	assert(res == VK_SUCCESS);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, surfFormats);
	assert(res == VK_SUCCESS);
	VkFormat format;

	{
		bool found = false;
		for(uint32_t i = 0; i < formatCount; i++)
		{
			if(surfFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				found = true;
				break;
			}
		}

		if(found)
			format = VK_FORMAT_R8G8B8A8_UNORM;
		else
			format = surfFormats[0].format;

	}

	delete[] surfFormats;

	//determine the 'extent' of the swapchain (the resolution/width/h)
	VkSurfaceCapabilitiesKHR surfCapabilities;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpus[0], surface, &surfCapabilities);
	assert(res == VK_SUCCESS);

	VkExtent2D swapchainExtent = surfCapabilities.currentExtent;
	printf("w:%d h:%d\n", surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height);
	//if sdl didn't set the extents (which it does...)
	
	//if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
	//	swapchainExtent.width = myWidth;
	//	swapchainExtent.height = myHeight;
	//	if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
	//		swapchainExtent.width = surfCapabilities.minImageExtent.width;
	//	} else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
	//		swapchainExtent.width = surfCapabilities.maxImageExtent.width;
	//	}

	//	if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
	//		swapchainExtent.height = surfCapabilities.minImageExtent.height;
	//	} else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
	//		swapchainExtent.height = surfCapabilities.maxImageExtent.height;
	//	}
	//} 
	
	
	
		//get surface present modes
		//uint32_t presentModeCount;
		//res = vkGetDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, nullptr);
		//assert(res == VK_SUCCESS);
		//VkPresentModeKHR* presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
		//res = vkGetDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, presentModes);
		//assert(res == VK_SUCCESS);

		//free(presentModes)
	

	// The FIFO present mode is guaranteed by the spec to be supported
	//this is why the previous code is commented out
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;


	// Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at at time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if(surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		preTransform = surfCapabilities.currentTransform;

	//find supported composite alpha mode- one is guarenteed to be sets
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for(uint32_t i = 0; i < sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); i++)
	{
		if(surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
		{
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = nullptr;
	swapchain_ci.surface = surface;
	swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
	swapchain_ci.imageFormat = format;
	swapchain_ci.imageExtent.width = swapchainExtent.width;
	swapchain_ci.imageExtent.height = swapchainExtent.height;
	swapchain_ci.preTransform = preTransform;
	swapchain_ci.compositeAlpha = compositeAlpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = swapchainPresentMode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = VK_TRUE;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = nullptr;


	//I'm not dealing with this complication
	//todo: deal with this
	//uint32_t queueFamilyIndices[2] = {(uint32_t)info.graphics_queue_family_index, (uint32_t)info.present_queue_family_index};
	//if (info.graphics_queue_family_index != info.present_queue_family_index) {
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between
		// the queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
	//	swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	//	swapchain_ci.queueFamilyIndexCount = 2;
	//	swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
	//}

	VkSwapchainKHR swap_chain;
	uint32_t swapchainImageCount;
	
	res = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swap_chain);
	assert(res == VK_SUCCESS);

	res = vkGetSwapchainImagesKHR(device, swap_chain, &swapchainImageCount, nullptr);
	assert(res == VK_SUCCESS);

	VkImage *swapchainImages = (VkImage*)malloc(swapchainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	res = vkGetSwapchainImagesKHR(device, swap_chain, &swapchainImageCount, swapchainImages);
	assert(res == VK_SUCCESS);


	std::vector<swap_chain_buffer> buffers(swapchainImageCount);
	for(uint32_t i = 0; i < swapchainImageCount; i++)
		buffers[i].image = swapchainImages[i];

	free(swapchainImages);


	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = nullptr;
		color_image_view.flags = 0;
		color_image_view.image = buffers[i].image;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		res = vkCreateImageView(device, &color_image_view, nullptr, &buffers[i].view);
		assert(res == VK_SUCCESS);
	}

	
	//end create swapchain-----------------------------------------------------

	//create depth buffer------------------------------------------------------

	struct {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

	VkImageCreateInfo image_info = {};
	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkFormatProperties fProps;
	vkGetPhysicalDeviceFormatProperties(gpus[0], depth_format, &fProps);
	if(fProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	else if(fProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	else	//try other formats?
		derror("VK_FORMAT_D16_UNORM unsupported!");


	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = nullptr;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = wWidth;
	image_info.extent.height = wHeight;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = nullptr;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = nullptr;
	mem_alloc.allocationSize = 0; //we will set these
	mem_alloc.memoryTypeIndex = 0;

	

	VkMemoryRequirements mem_reqs;
	depth.format = depth_format;
	res = vkCreateImage(device, &image_info, nullptr, &depth.image);
	assert(res == VK_SUCCESS);
	vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;
	VkPhysicalDeviceMemoryProperties memProps;
	//determine mem type
	{
		bool success = false;
		uint32_t typeBits = mem_reqs.memoryTypeBits;
		vkGetPhysicalDeviceMemoryProperties(gpus[0], &memProps);
		for(uint32_t i = 0; i < memProps.memoryTypeCount; i++)
		{
			if((typeBits & 1) == 1)
			{
				if((memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					mem_alloc.memoryTypeIndex = i;
					success = true;
					break;
				}
			}
			typeBits >>= 1;
		}
		assert(success);
	}

	res = vkAllocateMemory(device, &mem_alloc, nullptr, &depth.mem);
	assert(res == VK_SUCCESS);

	res = vkBindImageMemory(device, depth.image, depth.mem, 0);
	assert(res == VK_SUCCESS);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = nullptr;
	view_info.image = depth.image;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	res = vkCreateImageView(device, &view_info, nullptr, &depth.view);
	assert(res == VK_SUCCESS);


	//end create depth buffer--------------------------------------------------



	//create command buffer(the place where we put our commands)---------------
	//we make command calls to add commands to command buffer
	//we use pools to manage command buffers(since some short lived buffers could waste memory)
	//there is usually a pool per Queue Family, but we only made one.

	//create command pool

	
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = nullptr;
	cmd_pool_info.queueFamilyIndex = queue_info.queueFamilyIndex;
	cmd_pool_info.flags = 0;

	//we allocate our command buffer from this pool
	VkCommandPool cmd_pool;
	res = vkCreateCommandPool(device, &cmd_pool_info, nullptr, &cmd_pool);
	assert(res == VK_SUCCESS);

	//create command buffer from command pool


	VkCommandBufferAllocateInfo cmd_info = {};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandPool = cmd_pool;
	cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_info.commandBufferCount = 1; //we can do more than one at once if we want

	VkCommandBuffer cmd;
	res = vkAllocateCommandBuffers(device, &cmd_info, &cmd);
	assert(res == VK_SUCCESS);

	
	//end create command buffer/pool-------------------------------------------

	//create uniform buffer----------------------------------------------------

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(glm::vec3(-5, 3, -10), //camera loc in World Space
								 glm::vec3(0,0,0), //where camera looks
								 glm::vec3(0,-1,0) //head is up
								);

	glm::mat4 Model = glm::mat4(1.0f);

	//vulkan clip space inverts y and half z
	glm::mat4 Clip = glm::mat4 (1.0f, 0.0f, 0.0f, 0.0f,
								0.0f,-1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.5f, 0.0f,
								0.0f, 0.0f, 0.5f, 1.0f);

	glm::mat4 MVP = Clip * Projection * View * Model; //Model View Projection


	struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } uniform_data;


	
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = nullptr;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = sizeof(MVP);
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = nullptr;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;

	res = vkCreateBuffer(device, &buf_info, nullptr, &uniform_data.buf);
	

	



	

	//cleanup
	

	VkCommandBuffer cmd_bufs[1] = {cmd}; //we can free multiple
	vkFreeCommandBuffers(device, cmd_pool, 1, cmd_bufs); 
	vkDestroyCommandPool(device, cmd_pool, nullptr);
	
	//for(uint32_t i = 0; i < swapchainImageCount; i++)
	//	vkDestroyImageView(device, buffers[i].view, nullptr);

	//vkDestroySwapchainKHR(device, swap_chain, nullptr);

	vkDestroyImageView(device, depth.view, nullptr);
	vkDestroyImage(device, depth.image, nullptr);
	vkFreeMemory(device, depth.mem, nullptr);

	vkDestroyDevice(device, nullptr);
	DestroyDebugUtilsMessengerEXT(inst, debugMessenger, nullptr);
	vkDestroySurfaceKHR(inst, surface, nullptr);
	vkDestroyInstance(inst, nullptr);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
