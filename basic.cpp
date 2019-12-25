#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstdint>

/*
int main(int argc, char const *argv[])
{
	SDL_Window* window = SDL_CreateWindow("My App",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1280, 720,
    SDL_WINDOW_VULKAN);


	uint32_t extensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
	std::vector<const char *> extensionNames(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());

	VkApplicationInfo appInfo;
	// TODO: fill this out

	std::vector<const char *> layerNames;
	// uncomment below if you want to use validation layers
	layerNames.push_back("VK_LAYER_LUNARG_standard_validation");

	VkInstanceCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &appInfo;
	info.enabledLayerCount = layerNames.size();
	info.ppEnabledLayerNames = layerNames.data();
	info.enabledExtensionCount = extensionNames.size();
	info.ppEnabledExtensionNames = extensionNames.data();

	VkResult res;
	VkInstance instance;
	res = vkCreateInstance(&info, nullptr, &instance);
	if (res != VK_SUCCESS) {
	  // do some error checking
	}

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
	  // failed to create a surface!
	}

	while(true)
	{

	}



	return 0;
}
*/


/*
Vulkan Steps

1. Create An instance
2. Create a logical Device
	a. enumerate physical devices and pick one
	b. grab the right queue_family for the job, from the device
	c. make the device
3. Create a command buffer to the logical device
	a. Make a command pool(manages memory for command buffers)
		i. make sure the command pool is to the right queue family and device
	b. make a command buffer from the pool
4. 

*/


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


int main(int argc, char const *argv[])
{
	//create an sdl window
	SDL_Window* window;
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("My App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
	if(window == NULL)
		derror(std::string("Could not create window: ") + SDL_GetError());
	
	uint32_t extensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
	std::vector<const char *> extensionNames(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());


	//creating instance--------------------------------------------------------
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Vulkan Test";
	app_info.applicationVersion = 0;
	app_info.pEngineName = "Expert Test";
	app_info.engineVersion = 0;
	app_info.apiVersion = VK_API_VERSION_1_0; //vulkan 1.0 or 1.1?


	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = extensionNames.size();
	inst_info.ppEnabledExtensionNames = extensionNames.data();
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = nullptr;

	VkInstance inst;
	
	VkResult res = vkCreateInstance(&inst_info, nullptr, &inst);
	if(res == VK_ERROR_INCOMPATIBLE_DRIVER)
		derror("Could not find a compatible Vulkan ICD!\n");
	else if(res)
		derror("Unknown error!\n");
	

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

	//first we need the right device queue

	VkDeviceQueueCreateInfo queue_info = {};
	uint32_t queue_family_count = 0;
	std::vector<VkQueueFamilyProperties> queue_props;

	//get device queue props
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, nullptr);
	assert(queue_family_count >= 1);
	queue_props.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, queue_props.data());

	//find the 'graphics bit' in the properties to ensure that we can do graphics w/ this device queue (obv)
	//this is because a device can have multiple queues w/ different purposes
	//ie determine the queue_family index we want
	bool found = false;
	for(unsigned i = 0; i < queue_family_count; i++)
	{
		if(queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queue_info.queueFamilyIndex = i; 
			found = true;
			break;
		}
	}

	assert(found);
	assert(queue_family_count >= 1);


	float queue_priorities[1] = {0.0}; //this is only for dealing w/ multiple queues, so we don't care
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = nullptr;
	queue_info.queueCount = 1; //like i said, you can use multiple queues
	queue_info.pQueuePriorities = queue_priorities; 


	//now we can create a logical device using the queue we found

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = nullptr;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = nullptr;
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = nullptr;
	device_info.pEnabledFeatures = nullptr;

	VkDevice device;
	res = vkCreateDevice(gpus[0], &device_info, nullptr, &device);
	assert(res == VK_SUCCESS);


	//end device intialization-------------------------------------------------

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

	//create swap-chain/windowing related crap--------------------------------- 

	VkSurfaceKHR surface;
	if(!SDL_Vulkan_CreateSurface(window, inst, &surface))
		derror(std::string("Could not create window: ") + SDL_GetError());

	// we need to make sure that we 





	//cleanup
	SDL_DestroyWindow(window);

	VkCommandBuffer cmd_bufs[1] = {cmd}; //we can free multiple
	vkFreeCommandBuffers(device, cmd_pool, 1, cmd_bufs); 
	vkDestroyCommandPool(device, cmd_pool, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(inst, nullptr);


	SDL_Quit();
	return 0;
}
