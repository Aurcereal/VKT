
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN // GLFW auto-loads Vulkan header
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

using namespace std;
using namespace vk::raii;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

class Application {
public:
    void Run() {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    // raii library so it'll do the cleanup for us
    GLFWwindow* window = nullptr;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;

    Device device = nullptr;
    Queue graphicsQueue = nullptr;
    vector<const char*> desiredDeviceExtensions = {
        vk::KHRSwapchainExtensionName
    };

    const vector<const char*> desiredValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG // Not Debug, Part of C++ Standard
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // GLFW was for OpenGL, tell it don't create OpenGL Context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanTESTT", nullptr, nullptr); // 4th param is Monitor
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
        // messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

        return vk::False;
    }

    vector<const char*> GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers) {
            // Need this additional extension for a callback
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        return extensions;
    }

    void SetupDebugMessenger() {
        // Debug callback for validation layers so they can tell us what went wrong
        if (!enableValidationLayers) return;

        // When should our callback be called?
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError); // Only the bad ones
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation); // Want all message types
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
            .messageSeverity = severityFlags,
            .messageType = messageTypeFlags,
            .pfnUserCallback = &debugCallback
        };
        // There are more ways to configure callback, look at Validation Layers article bottom
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    }

    void CreateInstance() {
        // Required validation Layers
        std::vector<char const*> requiredLayers;
        if (enableValidationLayers) {
            // Copy from begin to end
            requiredLayers.assign(desiredValidationLayers.begin(), desiredValidationLayers.end());
        }
        
        // Check if required validation layers are supported
        auto layerProperties = context.enumerateInstanceLayerProperties();
        for (const auto& requiredLayer : requiredLayers) {
            bool found = false;
            for (const auto& supportedLayer : layerProperties) {
                std::cout << "Supported Layer: " << supportedLayer.layerName << std::endl;
                if (strcmp(supportedLayer.layerName, requiredLayer) == 0)
                    found = true;
            }
            if (!found) {
                throw std::runtime_error("Required validation layer not supported!");
            }
        }

        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Vulkan TEST APP",
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "No Engine",
            .apiVersion = vk::ApiVersion14
        };

        // Desired Extensions
        auto extensions = GetRequiredExtensions();

        // Supported Extensions
        auto supportedExtensions = context.enumerateInstanceExtensionProperties();
        cout << "Supported Extensions:\n";
        for (const auto& extension : supportedExtensions) {
            cout << "\t" << extension.extensionName << endl;
        }
        
        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };
		
        try {
            instance = vk::raii::Instance(context, createInfo);
        }
        catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
            return;
        }
        catch (const std::exception& err) {
            std::cerr << "Error: " << err.what() << std::endl;
        }

    }

    static bool IsDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) {
        auto deviceProperties = physicalDevice.getProperties();
        auto deviceFeatures = physicalDevice.getFeatures(); // What optional features?
        
        // Must support API Version >= 1.3
        bool isSuitable = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_3;

        // It must have a queue family that supports graphics calls
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        
        bool foundGraphicsFamily = false;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                foundGraphicsFamily = true;
                break;
            }
        }
        isSuitable = isSuitable && foundGraphicsFamily;
        return isSuitable;
    }

    static uint32_t FindGraphicsQueueFamilyIndex(const PhysicalDevice& physicalDevice) {
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        auto graphicsIter = std::ranges::find_if(queueFamilies, [&](const auto& qFamily) {
            return (qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
            });
        assert(graphicsIter != queueFamilies.end());
        return static_cast<uint32_t>(std::distance(queueFamilies.begin(), graphicsIter));
    }

    void PickPhysicalDevice() {
        // List graphics cards
        vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
        if (devices.empty()) {
            throw std::runtime_error("No GPUs that support vulkan");
        }
        // We gotta manually go through each device whereas WebGPU we could just say what features we want mostly
        const auto devIter = std::ranges::find_if(devices, IsDeviceSuitable);
        if (devIter == devices.end()) {
            throw std::runtime_error("Couldn't find suitable GPU!");
        }
        physicalDevice = *devIter;
    }

    void CreateLogicalDevice() {
        uint32_t graphicsIndex = FindGraphicsQueueFamilyIndex(physicalDevice);
        float queuePriority = 0.5f; // [0,1] mandatory even if 1 queue
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ 
            .queueFamilyIndex = graphicsIndex, // Index within physical device
            .queueCount = 1, 
            .pQueuePriorities = &queuePriority }; // For creating the graphics queue
        // We can only have a few queues per queue family, and we only really need one per family.  We can create cmd buffers on multiple threads and submit all of them on main thread with low overhead
        
        // Modern simplification to auto chain structs with pNext
        vk::StructureChain<
            vk::PhysicalDeviceFeatures2, // A
            vk::PhysicalDeviceVulkan13Features, // B 
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT // C
        >
            featureChain = {
                {}, // constructor for A
                {.dynamicRendering = true}, // constructor for B, dynamic rendering is a modern simplification
                {.extendedDynamicState = true}
        };

        vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(), // The pointer to the first in the chain
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = desiredDeviceExtensions.data() // Device specific extensions
        }; // Device specific validation layers obsolete so not here

        device = Device(physicalDevice, deviceCreateInfo);
        graphicsQueue = Queue(device, graphicsIndex, 0); // Get queue from our new device, 0 is the queue index within family, only 1 queue so we put 0

    }

    void InitVulkan() {
        CreateInstance();
        SetupDebugMessenger();
        PickPhysicalDevice();
        CreateLogicalDevice();
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // Check for User Input Events

        }
    }

    void Cleanup() {
        // GLFW
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    Application app;

    try {
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}