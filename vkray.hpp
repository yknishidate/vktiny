
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <GLFW/glfw3.h>

namespace vkray
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class Context
    {
    public:
        void init(const bool enableValidation, const char* appName);
        bool shouldStop();
        void processEvents();
        ~Context();

        GLFWwindow* window;
        uint32_t width{ 1280 };
        uint32_t height{ 720 };

        vk::UniqueInstance instance;
        vk::UniqueSurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        QueueFamilyIndices queueFamilyIndices;
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
        vk::Queue graphicsQueue;

        bool enableValidation;
        const char* appName;

    private:
        vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
        };


        void createWindow();
        void createInstance();
        void createDebugMessenger();
        void createSurface();
        void createLogicalDevice();
        void getGraphicsQueue();

        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        vk::PhysicalDevice pickPhysicalDevice();
        bool isDeviceSuitable(const vk::PhysicalDevice& device);
        void findQueueFamilies(const vk::PhysicalDevice& device);
        bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);
        SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT*, void*);
    };


    void Context::init(const bool enableValidation, const char* appName)
    {
        this->enableValidation = enableValidation;
        this->appName = appName;

        createWindow();
        createInstance();
        createDebugMessenger();
        createSurface();
        createLogicalDevice();
        getGraphicsQueue();
    }

    inline bool Context::shouldStop()
    {
        return !glfwWindowShouldClose(window);
    }

    inline void Context::processEvents()
    {
        glfwPollEvents();
    }

    inline Context::~Context()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Context::createWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, appName, nullptr, nullptr);
    }

    void Context::createInstance()
    {
        // インスタンスに依存しない関数ポインタを取得する
        static vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        if (enableValidation && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo appInfo{
            appName,
            VK_MAKE_VERSION(1, 0, 0),
            "Engine",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2 };

        auto extensions = getRequiredExtensions();

        if (enableValidation) {
            // デバッグモードの場合
            vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

            vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

            vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> createInfo{
                { {}, &appInfo, validationLayers, extensions },
                { {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback } };
            instance = vk::createInstanceUnique(createInfo.get<vk::InstanceCreateInfo>());
        } else {
            // リリースモードの場合
            vk::InstanceCreateInfo createInfo{ {}, &appInfo, {}, extensions };
            instance = vk::createInstanceUnique(createInfo, nullptr);
        }

        // 全ての関数ポインタを取得する
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
    }

    void Context::createDebugMessenger()
    {
        if (!enableValidation) {
            return;
        }

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{
            {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback };

        debugUtilsMessenger = instance->createDebugUtilsMessengerEXTUnique(createInfo);
    }

    void Context::createSurface()
    {
        // glfw は生の VkSurface や VkInstance で操作する必要がある
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(VkInstance(instance.get()), window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance.get());
        surface = vk::UniqueSurfaceKHR{ vk::SurfaceKHR(_surface), _deleter };
    }

    void Context::createLogicalDevice()
    {
        physicalDevice = pickPhysicalDevice();
        physicalDeviceMemoryProperties = physicalDevice.getMemoryProperties();

        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{ {}, queueFamilyIndices.graphicsFamily.value(), 1, &queuePriority };

        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo
            .setQueueCreateInfos(queueCreateInfo)
            .setPEnabledExtensionNames(deviceExtensions);
        if (enableValidation) {
            deviceCreateInfo.setPEnabledLayerNames(validationLayers);
        }

        vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR> deviceProperties =
            physicalDevice.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
        rayTracingPipelineProperties = deviceProperties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

        vk::PhysicalDeviceFeatures2 features2 = physicalDevice.getFeatures2();

        vk::StructureChain<
            vk::DeviceCreateInfo,
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceBufferDeviceAddressFeatures> createInfoChain{
                { deviceCreateInfo }, { features2 }, { VK_TRUE }, { VK_TRUE }, { VK_TRUE } };

        device = physicalDevice.createDeviceUnique(createInfoChain.get<vk::DeviceCreateInfo>());

        VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());
    }

    inline void Context::getGraphicsQueue()
    {
        graphicsQueue = device->getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL
        Context::debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
            void* /*pUserData*/)
    {
        std::cerr << "messageIDName   = " << pCallbackData->pMessageIdName << "\n";
        for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
            std::cerr << "objectType      = "
                << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
        }
        std::cerr << pCallbackData->pMessage << "\n";
        std::cerr << "\n";
        return VK_FALSE;
    }

    bool Context::checkValidationLayerSupport()
    {
        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        const char* layerName = "VK_LAYER_KHRONOS_validation";
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                return true;
            }
        }
        return false;
    }

    std::vector<const char*> Context::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidation) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    inline vk::PhysicalDevice Context::pickPhysicalDevice()
    {
        // 全ての物理デバイスを取得
        std::vector<vk::PhysicalDevice> devices = instance->enumeratePhysicalDevices();

        // 適切な物理デバイスを選択
        vk::PhysicalDevice physicalDevice;
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        return physicalDevice;
    }

    inline bool Context::isDeviceSuitable(const vk::PhysicalDevice& device)
    {
        findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    inline void Context::findQueueFamilies(const vk::PhysicalDevice& device)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                queueFamilyIndices.graphicsFamily = i;
            }

            VkBool32 presentSupport = device.getSurfaceSupportKHR(i, surface.get());
            if (presentSupport) {
                queueFamilyIndices.presentFamily = i;
            }

            if (queueFamilyIndices.isComplete()) {
                break;
            }

            i++;
        }
    }

    inline bool Context::checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
    {
        std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

        std::set<std::string> requiredExtensions{ deviceExtensions.begin(), deviceExtensions.end() };

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (!requiredExtensions.empty()) {
            return false;
        }

        std::cout << "Check Device Extension Support: All OK" << std::endl;
        for (auto& extension : deviceExtensions) {
            std::cout << "    " << extension << std::endl;
        }

        return true;
    }

    inline SwapChainSupportDetails Context::querySwapChainSupport(vk::PhysicalDevice device)
    {
        SwapChainSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface.get());
        details.formats = device.getSurfaceFormatsKHR(surface.get());
        details.presentModes = device.getSurfacePresentModesKHR(surface.get());

        return details;
    }

}
