#pragma once
#include "DebugMessenger.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

class Context
{
public:
    ~Context()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initialize(uint32_t apiVersion, bool enableValidationLayer,
                    int width, int height,
                    std::vector<const char*> deviceExtensions = {},
                    void* deviceCreatePNext = nullptr)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);

        // instance extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // layers
        std::vector<const char*> layers;
        if (enableValidationLayer) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        instance.initialize(VK_API_VERSION_1_2, layers, instanceExtensions);
        messenger.initialize(instance);
        surface.initialize(instance, window);
        physicalDevice.initialize(instance);
        device.initialize(instance, physicalDevice, surface, layers,
                          deviceExtensions, deviceCreatePNext);
        swapchain.initialize(device, physicalDevice, surface, width, height);
    }

    bool running() const
    {
        return !glfwWindowShouldClose(window);
    }

    void pollEvents() const
    {
        glfwPollEvents();
    }

    auto& getSwapchain() { return swapchain; }
    const auto& getDevice() const { return device; }
    const auto& getPhysicalDevice() const { return physicalDevice; }

private:
    GLFWwindow* window;

    Instance instance;
    DebugMessenger messenger;
    Surface surface;
    PhysicalDevice physicalDevice;
    Device device;
    Swapchain swapchain;
};
