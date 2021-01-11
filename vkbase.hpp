#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <algorithm>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkray
{
    class Window;
    class Instance;
    class Device;

    class Window final
    {
    public:
        Window(const std::string& title, const uint32_t width, const uint32_t height,
            bool cursorDisabled = false, bool fullscreen = false, bool resizable = false);

        ~Window()
        {
            if (window != nullptr) {
                glfwDestroyWindow(window);
                window = nullptr;
            }

            glfwTerminate();
            glfwSetErrorCallback(nullptr);
        }

        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator = (const Window&) = delete;
        Window& operator = (Window&&) = delete;

        GLFWwindow* getHandle() const { return window; }

        float getContentScale() const
        {
            float xscale;
            float yscale;
            glfwGetWindowContentScale(window, &xscale, &yscale);

            return xscale;
        }

        vk::Extent2D getFramebufferSize() const
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        vk::Extent2D getWindowSize() const
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        const char* getKeyName(int key, int scancode) const { return glfwGetKeyName(key, scancode); }

        std::vector<const char*> getRequiredInstanceExtensions() const
        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
        }

        double getTime() const { return glfwGetTime(); }

        std::string getTitle() const { return title; }

        // Callbacks
        std::function<void()> drawFrame;
        std::function<void(int key, int scancode, int action, int mods)> onKey;
        std::function<void(double xpos, double ypos)> onCursorPosition;
        std::function<void(int button, int action, int mods)> onMouseButton;
        std::function<void(double xoffset, double yoffset)> onScroll;

        // Methods
        void close() { glfwSetWindowShouldClose(window, 1); }

        bool isMinimized() const
        {
            const auto size = getFramebufferSize();
            return size.height == 0 && size.width == 0;
        }

        void run()
        {
            glfwSetTime(0.0);

            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();

                if (drawFrame) {
                    drawFrame();
                }
            }
        }

        void waitForEvents() const { glfwWaitEvents(); }

    private:

        GLFWwindow* window{};

        std::string title;
        uint32_t width;
        uint32_t height;
        bool cursorDisabled;
        bool fullscreen;
        bool resizable;
    };


    class Instance final
    {
    public:
        Instance(const Window& window, const bool enableValidationLayers);
        ~Instance() {}

        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator = (const Instance&) = delete;
        Instance& operator = (Instance&&) = delete;

        vk::Instance getHandle() const { return *instance; }
        const Window& getWindow() const { return window; }

        const std::vector<vk::ExtensionProperties>& getExtensions() const { return extensions; }
        const std::vector<vk::PhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }
        const std::vector<const char*>& getValidationLayers() const { return validationLayers; }

        vk::UniqueSurfaceKHR createSurface() const
        {
            VkSurfaceKHR _surface;

            if (glfwCreateWindowSurface(VkInstance(*instance), window.getHandle(), nullptr, &_surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

            vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(*instance);

            return vk::UniqueSurfaceKHR{ vk::SurfaceKHR(_surface), _deleter };
        }

        vk::PhysicalDevice pickSuitablePhysicalDevice() const
        {
            const auto result = std::find_if(physicalDevices.begin(), physicalDevices.end(), [](const vk::PhysicalDevice& device) {
                // We want a device with a graphics queue.
                const auto queueFamilies = device.getQueueFamilyProperties();

                const auto hasGraphicsQueue = std::find_if(queueFamilies.begin(), queueFamilies.end(), [](const VkQueueFamilyProperties& queueFamily) {
                    return queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                });

                return hasGraphicsQueue != queueFamilies.end();
            });

            if (result == physicalDevices.end()) {
                throw std::runtime_error("cannot find a suitable device");
            }

            return *result;
        }

    private:
        void getVulkanDevices()
        {
            physicalDevices = instance->enumeratePhysicalDevices();

            if (physicalDevices.empty()) {
                throw std::runtime_error("found no Vulkan physical devices");
            }
        }

        void getVulkanExtensions()
        {
            extensions = vk::enumerateInstanceExtensionProperties();
        }

        void createDebugMessenger();

        static void checkVulkanMinimumVersion(uint32_t minVersion)
        {
            uint32_t version = vk::enumerateInstanceVersion();
            if (minVersion > version) {
                throw std::runtime_error("minimum required version not found");
            }
        }

        static void checkVulkanValidationLayerSupport(const std::vector<const char*>& validationLayers)
        {
            const auto availableLayers = vk::enumerateInstanceLayerProperties();

            for (const char* layer : validationLayers) {
                auto result = std::find_if(availableLayers.begin(), availableLayers.end(), [layer](const VkLayerProperties& layerProperties) {
                    return strcmp(layer, layerProperties.layerName) == 0;
                });

                if (result == availableLayers.end()) {
                    throw std::runtime_error("could not find the requested validation layer: '" + std::string(layer) + "'");
                }
            }
        }

        vk::UniqueInstance instance{};
        const Window& window;
        bool enableValidationLayers;
        std::vector<const char*> validationLayers;

        std::vector<vk::PhysicalDevice> physicalDevices;
        std::vector<vk::ExtensionProperties> extensions;

        vk::UniqueDebugUtilsMessengerEXT messenger;

    };


    class Device final
    {
    public:
        explicit Device(const Instance& instance);
        ~Device() {}

        Device(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator = (const Device&) = delete;
        Device& operator = (Device&&) = delete;

        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        vk::SurfaceKHR getSurface() const { return *surface; }

        uint32_t getGraphicsFamilyIndex() const { return graphicsFamilyIndex; }
        uint32_t getComputeFamilyIndex() const { return computeFamilyIndex; }
        uint32_t getPresentFamilyIndex() const { return presentFamilyIndex; }
        uint32_t getTransferFamilyIndex() const { return transferFamilyIndex; }
        vk::Queue getGraphicsQueue() const { return graphicsQueue; }
        vk::Queue getComputeQueue() const { return computeQueue; }
        vk::Queue getPresentQueue() const { return presentQueue; }
        vk::Queue getTransferQueue() const { return transferQueue; }

        void waitIdle() const;

    private:

        void checkRequiredExtensions(vk::PhysicalDevice physicalDevice) const
        {
            const auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

            std::set<std::string> requiredExtensions(requiredExtensions.begin(), requiredExtensions.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            if (!requiredExtensions.empty()) {
                bool first = true;
                std::string extensions;

                for (const auto& extension : requiredExtensions) {
                    if (!first) {
                        extensions += ", ";
                    }

                    extensions += extension;
                    first = false;
                }

                throw std::runtime_error("missing required extensions: " + extensions);
            }
        }

        const std::vector<const char*> requiredExtensions{
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

        const Instance& instance;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueSurfaceKHR surface;

        vk::UniqueDevice device;

        uint32_t graphicsFamilyIndex{};
        uint32_t computeFamilyIndex{};
        uint32_t presentFamilyIndex{};
        uint32_t transferFamilyIndex{};

        vk::Queue graphicsQueue{};
        vk::Queue computeQueue{};
        vk::Queue presentQueue{};
        vk::Queue transferQueue{};
    };


    namespace
    {
#if defined(_DEBUG)
        VKAPI_ATTR VkBool32 VKAPI_CALL
            debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* /*pUserData*/)
        {
            std::cerr << "messageIDName   = " << pCallbackData->pMessageIdName << "\n";

            for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
                std::cerr << "objectType      = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
            }

            std::cerr << pCallbackData->pMessage << "\n\n";

            return VK_FALSE;
        }
#endif

        void glfwErrorCallback(const int error, const char* const description)
        {
            std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
        }

        void glfwKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onKey) {
                this_->onKey(key, scancode, action, mods);
            }
        }

        void glfwCursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onCursorPosition) {
                this_->onCursorPosition(xpos, ypos);
            }
        }

        void glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onMouseButton) {
                this_->onMouseButton(button, action, mods);
            }
        }

        void glfwScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onScroll) {
                this_->onScroll(xoffset, yoffset);
            }
        }

        std::vector<vk::QueueFamilyProperties>::const_iterator findQueue(
            const std::vector<vk::QueueFamilyProperties>& queueFamilies,
            const std::string& name,
            const vk::QueueFlags requiredBits,
            const vk::QueueFlags excludedBits)
        {
            const auto family = std::find_if(queueFamilies.begin(), queueFamilies.end(), [requiredBits, excludedBits](const vk::QueueFamilyProperties& queueFamily) {
                return queueFamily.queueCount > 0 && queueFamily.queueFlags & requiredBits && !(queueFamily.queueFlags & excludedBits);
            });

            if (family == queueFamilies.end()) {
                throw std::runtime_error("found no matching " + name + " queue");
            }

            return family;
        }

    } // namespace

    ////////////////////
    // implementation //
    ////////////////////

    Window::Window(const std::string& title, const uint32_t width, const uint32_t height, bool cursorDisabled, bool fullscreen, bool resizable)
        : title(title), width(width), height(height), cursorDisabled(cursorDisabled), fullscreen(fullscreen), resizable(resizable)
    {
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("glfwInit() failed");
        }

        if (!glfwVulkanSupported()) {
            throw std::runtime_error("glfwVulkanSupported() failed");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        auto* const monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
        if (window == nullptr) {
            throw std::runtime_error("failed to create window");
        }

        if (cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
    }

    Instance::Instance(const Window& window, const bool enableValidationLayers)
        : window(window)
        , enableValidationLayers(enableValidationLayers)
    {
        static vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        validationLayers = enableValidationLayers ? std::vector<const char*>{"VK_LAYER_KHRONOS_validation"} : std::vector<const char*>();

        const uint32_t version = VK_API_VERSION_1_2;

        checkVulkanMinimumVersion(version);

        auto extensions = window.getRequiredInstanceExtensions();

        checkVulkanValidationLayerSupport(validationLayers);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = window.getTitle().c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        instance = vk::createInstanceUnique(createInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        getVulkanDevices();
        getVulkanExtensions();

        if (enableValidationLayers) {
            createDebugMessenger();
        }
    }

    void Instance::createDebugMessenger()
    {
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{ {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback };

        messenger = instance->createDebugUtilsMessengerEXTUnique(createInfo);
    }

    Device::Device(const Instance& instance)
        : instance(instance)
    {
        surface = instance.createSurface();
        physicalDevice = instance.pickSuitablePhysicalDevice();

        checkRequiredExtensions(physicalDevice);

        const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        // Find the graphics queue.
        const auto graphicsFamily = findQueue(queueFamilies, "graphics", vk::QueueFlagBits::eGraphics, {});
        const auto computeFamily = findQueue(queueFamilies, "compute", vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
        const auto transferFamily = findQueue(queueFamilies, "transfer", vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);

        // Find the presentation queue (usually the same as graphics queue).
        const auto presentFamily = std::find_if(queueFamilies.begin(), queueFamilies.end(), [&](const vk::QueueFamilyProperties& queueFamily) {
            VkBool32 presentSupport = false;
            const uint32_t i = static_cast<uint32_t>(&*queueFamilies.cbegin() - &queueFamily);
            presentSupport = physicalDevice.getSurfaceSupportKHR(i, *surface);
            return queueFamily.queueCount > 0 && presentSupport;
        });

        if (presentFamily == queueFamilies.end()) {
            throw std::runtime_error("found no presentation queue");
        }

        graphicsFamilyIndex = static_cast<uint32_t>(graphicsFamily - queueFamilies.begin());
        computeFamilyIndex = static_cast<uint32_t>(computeFamily - queueFamilies.begin());
        presentFamilyIndex = static_cast<uint32_t>(presentFamily - queueFamilies.begin());
        transferFamilyIndex = static_cast<uint32_t>(transferFamily - queueFamilies.begin());

        // Queues can be the same
        const std::set<uint32_t> uniqueQueueFamilies =
        {
            graphicsFamilyIndex,
            computeFamilyIndex,
            presentFamilyIndex,
            transferFamilyIndex
        };

        // Create queues
        float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        for (uint32_t queueFamilyIndex : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // TODO: add raytracing features
        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = true;
        deviceFeatures.samplerAnisotropy = true;

        vk::PhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
        indexingFeatures.runtimeDescriptorArray = true;

        vk::DeviceCreateInfo createInfo{};
        createInfo.pNext = &indexingFeatures;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledLayerCount = static_cast<uint32_t>(instance.getValidationLayers().size());
        createInfo.ppEnabledLayerNames = instance.getValidationLayers().data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        device = physicalDevice.createDeviceUnique(createInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

        graphicsQueue = device->getQueue(graphicsFamilyIndex, 0);
        computeQueue = device->getQueue(computeFamilyIndex, 0);
        presentQueue = device->getQueue(presentFamilyIndex, 0);
        transferQueue = device->getQueue(transferFamilyIndex, 0);
    }

} // vkf
