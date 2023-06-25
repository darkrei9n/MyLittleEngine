#pragma once


#include "VKPipeLineBuild.h"
#include "../../../Resource Management/Headers/vk_mem_alloc.h"



typedef struct {
	VkLayerProperties properties;
	std::vector<VkExtensionProperties> instance_extensions;
	std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

typedef struct VKQueues {
public:
    char name;
    uint32_t queueFamilyIndex;
    VkQueue queue;

    VKQueues() {
        name = ' ';
        queueFamilyIndex = -1;
        queue = VK_NULL_HANDLE;
    }
};

typedef struct {

} VKCreatedImages;

typedef struct {
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkExtensionProperties> extensionProperties;
    std::vector<VkLayerProperties> layerProperties;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
} gpu;

typedef struct {
    //Our access to Vulkan and our acces to the device!
    VkInstance instance;
    VkDevice device;
    //The GPU we actually picked.
    gpu gpuInfo;
    //The window we need to render to.
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormatClass;

    //Memory Allocator
    VmaAllocator allocator;

    //Temporary Device Stuff
    std::vector<VkPhysicalDevice> physicalDevice;
    std::vector<VkPhysicalDeviceProperties> deviceProperties;
    std::vector<VkExtensionProperties> deviceExtensions;
    std::vector<VkPhysicalDeviceMemoryProperties> deviceMemoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;


}VKInfo;

typedef struct {
    VkDebugUtilsMessengerEXT debugMessenger;
} VKDebug;

typedef struct {
    std::shared_ptr<HWND> hWnd;
    std::shared_ptr<HINSTANCE> hInstance;
} WNDInfo;

struct AllocatedBuffer {
	VkBuffer buffer;
    VkImage image;
	VmaAllocation allocation;
    Matrix44 modelMatrix;
    std::vector<VertexStruct> vertexData;
    std::vector<uint32_t> indices;
};




class VulkanRender : public APIBase
{

    //Winda Objects
    WNDInfo wndInfo;
    //Global Object
    std::shared_ptr<GlobalManager> globalManager;

    //Debug stuff. Sigh.....
    #ifdef _DEBUG
	const bool enableValidationLayers = true;
    #else
    const bool enableValidationLayers = false;
    #endif
	//Vulkan Objects
    VKInfo vkInfo;
    VKDebug debug;

    //Vulkan Pipelines
    VkPipelineBuild pipelineBuilder;
    std::vector<Pipelines> pipelines;

    //Vulkan GPU
    gpu gpuInfo;

    //Memory Allocator
    std::vector<AllocatedBuffer> allocation;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkImage> images;
    
    VkPresentModeKHR presentModeClass;
    VkExtent2D swapChainExtentClass;

    VkQueue graphicsQueue;
    
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    //std::vector<Vertex3> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    VkFormat depthFormat;
    VkFormat colorFormat;

    //Temporary Shader Modules
    VkShaderModule vertShaderModule;   
    VkShaderModule fragShaderModule;
    std::vector<VkShaderModule> shaderModules;
    std::vector<Vector3> vertexData;

    //Buffer view
    VkBufferView bufferView;

    //Tempory... again...
    Camera *camera;

	//Vulkan Initialization
	void createInstance();
	void createSurface();
	void enumerateDevices();
    void selectDevice();
	void createLogicalDevice(int graphicsId);
    void createAllocator();
    void createSemaphores();
	void createCommandPool();
    void createCommandBuffers();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& capabilities, int height, int width);

    void createSwapChain(int height, int width);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

    void createRenderTargets();
    void createRenderPass();
    void createFrameBuffers();

    //Support Functions
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createImageViews(VkImage image, VkFormat format, VkImageAspectFlags flags);

    uint32_t findMemoryResources(uint32_t memoryRequirements, VkMemoryPropertyFlags properties);

    bool allocateMemory(VkDeviceMemory& memory, uint32_t memoryRequirements, uint32_t memoryTypeIndex);
    void processShader(std::vector<char> data);


    void preRender(VkCommandBuffer &commands, uint32_t &imageIndex);
    void postRender(VkCommandBuffer &commands, uint32_t &imageIndex);
public:

	VulkanRender(std::shared_ptr<HWND> hWnd, std::shared_ptr<HINSTANCE> hInstance, std::shared_ptr<GlobalManager> globalManager) {
        wndInfo.hWnd = hWnd;
        wndInfo.hInstance = hInstance;
        this->globalManager = globalManager;
	}

	void InitAPI(int width, int height) override;
    void LoadShader(const char* path);
    void LoadVertexData(const char* path);
    void buildPipeline(std::string name);
    void Render() override;
	void End() override;

};

